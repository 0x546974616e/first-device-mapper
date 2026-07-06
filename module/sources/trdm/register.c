#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/device-mapper.h>

#include "trdm/printk.h"
#include "trdm/register.h"

struct trdm_context {
  struct dm_dev* dev;
  struct mutex bio_alloc_lock;
  struct bio_set bs;
  sector_t start;
  char key;
};

struct trdm_bio {
  struct bio* original_bio;
  struct bvec_iter saved_iter;

  /// This member must come last, `bio_alloc_bioset()` will allocate enough
  /// bytes for entire `trdm_bio` but it relies on `struct bio` being last.
  struct bio bio;
};

static int trdm_ctr(struct dm_target* ti, unsigned int argc, char** argv) {
  if (argc < 3) {
    TRDM_ERRORLN("Invalid argument count (%i < 3).", argc);
    ti->error = "Invalid argument count";
    return -EINVAL;
  }

  struct trdm_context* tr;
  tr = kzalloc(sizeof(*tr), GFP_KERNEL);
  if (tr == NULL) {
    TRDM_ERRORLN("kzalloc(trdm_context) failed.");
    ti->error = "Cannot allocate context";
    return -ENOMEM;
  }

  mutex_init(&tr->bio_alloc_lock);

  int error = TRDM_SUCCESS;
  unsigned int front_pad = offsetof(struct trdm_bio, bio);
  if (bioset_init(&tr->bs, BIO_POOL_SIZE, front_pad, BIOSET_NEED_BVECS)) {
    TRDM_ERRORLN("bioset_init() failed.");
    ti->error = "Cannot initialise bioset";
    error = -ENOMEM;
    goto cleanup;
  }

  unsigned long long start;
  if (sscanf(argv[1], "%llu", &start) != 1 || start != (sector_t) start) {
    TRDM_ERRORLN("Invalid device sector `%s`.", argv[1]);
    ti->error = "Invalid device sector";
    error = -EINVAL;
    goto cleanup;
  }

  unsigned long long key;
  if (sscanf(argv[2], "%llu", &key) != 1 || key != (char) key) {
    TRDM_ERRORLN("Invalid cipher key `%s`.", argv[2]);
    ti->error = "Invalid cipher key";
    error = -EINVAL;
    goto cleanup;
  }

  error = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &tr->dev);
  if (error) {
    TRDM_ERRORLN("dm_get_device(argv[0] = %s) failed.", argv[0]);
    ti->error = "Device lookup failed";
    goto cleanup;
  }

  tr->key = (char) key;
  tr->start = (sector_t) start;
  TRDM_INFOLN("sector=%llu; key=%d", tr->start, tr->key);
  ti->private = tr;
  return TRDM_SUCCESS;

cleanup:
  mutex_destroy(&tr->bio_alloc_lock);
  bioset_exit(&tr->bs);
  kfree(tr);
  return error;
}

static void trdm_dtr(struct dm_target* ti) {
  struct trdm_context* tr = ti->private;
  mutex_destroy(&tr->bio_alloc_lock);
  dm_put_device(ti, tr->dev);
  bioset_exit(&tr->bs);
  kfree(tr);
}

// https://github.com/torvalds/linux/blob/master/fs/btrfs/bio.c
// https://github.com/torvalds/linux/blob/master/drivers/md/dm-crypt.c
// https://elixir.bootlin.com/linux/v7.1.2/source/block/bio.c
// https://linuxvox.com/blog/the-bio-structure-in-the-linux-kernel/#block-io-containers-bio-sets

static void transform_bio(struct bio* bio, char mask) {
  struct bio_vec bvec;
  struct bvec_iter iter;

  bio_for_each_segment(bvec, bio, iter) {
    // TRDM_INFOLN("&bvec=%p", &bvec);
    char* data = bvec_kmap_local(&bvec); // folio ?
    for (unsigned int i = 0u; i < bvec.bv_len; ++i) {
      data[i] ^= mask;
    }
    kunmap_local(data);
  }
}

static void trdm_endio_read(struct bio* clone_bio) {
  struct trdm_context* trdm = clone_bio->bi_private;
  struct trdm_bio* trdm_bio = container_of(clone_bio, struct trdm_bio, bio);
  struct bio* original_bio = trdm_bio->original_bio;

  // Est-ce que c'est safe d'itérer le original_bio ?
  // Quid de si les couches en-dessous ont splitté le bio ?
  if (clone_bio->bi_status == BLK_STS_OK) {
    clone_bio->bi_iter = trdm_bio->saved_iter;
    TRDM_INFOLN("key=0x%X", trdm->key);
    transform_bio(clone_bio, trdm->key);
  }

  original_bio->bi_status = clone_bio->bi_status;
  bio_put(clone_bio); // Releases the bioset allocation.
  bio_endio(original_bio); // Complete the original.
}

static int trdm_map(struct dm_target* ti, struct bio* given_bio) {
  struct trdm_context* trdm = ti->private;
  bio_set_dev(given_bio, trdm->dev->bdev);
  given_bio->bi_iter.bi_sector = trdm->start +
    dm_target_offset(ti, given_bio->bi_iter.bi_sector);

  switch (bio_op(given_bio)) {
    case REQ_OP_READ:
      mutex_lock(&trdm->bio_alloc_lock);
      struct bio* clone_bio = bio_alloc_clone(
        trdm->dev->bdev, given_bio, GFP_NOWAIT, &trdm->bs);

      // Normalement bio_alloc_clone() ne retournera jamais de ERR_PTR() mais,
      // sait-on jamais, la fonction évoluera peut-être un jour et le fera.
      if (clone_bio == NULL || IS_ERR(clone_bio)) {
        given_bio->bi_status = BLK_STS_RESOURCE;
        bio_endio(given_bio);
        return DM_MAPIO_SUBMITTED;
      }

      struct trdm_bio* trdm_bio = container_of(
        clone_bio, struct trdm_bio, bio);
      trdm_bio->original_bio = given_bio;
      trdm_bio->saved_iter = given_bio->bi_iter;
      trdm_bio->bio.bi_private = trdm;
      trdm_bio->bio.bi_end_io = trdm_endio_read;
      // clone_bio->bi_iter.bi_sector = given_bio->bi_iter.bi_sector;
      submit_bio(clone_bio);
      mutex_unlock(&trdm->bio_alloc_lock);

      /// You took full ownership of the bio. You called submit_bio() yourself
      /// (or queued it internally). DM must not touch it again. This is what
      /// we return in the write path after submitting our clone.
      return DM_MAPIO_SUBMITTED;

    case REQ_OP_WRITE:
      return DM_MAPIO_KILL;

    case REQ_OP_FLUSH:
    case REQ_OP_DISCARD:
    case REQ_OP_SECURE_ERASE:
    case REQ_OP_WRITE_ZEROES:
      // You updated bi_bdev and bi_sector (and optionally chained bi_end_io),
      // and now you hand it back to DM core which will call submit_bio() for
      // you. The simple case — used in our read path.
      // return DM_MAPIO_REMAPPED;

      return DM_MAPIO_KILL;

    case REQ_OP_ZONE_APPEND:
    case REQ_OP_ZONE_OPEN:
    case REQ_OP_ZONE_CLOSE:
    case REQ_OP_ZONE_FINISH:
    case REQ_OP_ZONE_RESET:
    case REQ_OP_ZONE_RESET_ALL:
      // DM will call bio_io_error() on the bio right away. The bio is ended
      // with BLK_STS_IOERR and propagated up as an I/O error. Use this when
      // the situation is unrecoverable — bad mapping, unresolvable error, or
      // as we do: when our clone allocation fails and there's no point
      // retrying.
      return DM_MAPIO_KILL;

    default:
      return DM_MAPIO_KILL;
  }
}

static struct target_type trdm_target = {
  .name    = "trdm",
  .version = {1, 0, 0},
  .module  = THIS_MODULE,
  .ctr     = trdm_ctr,
  .dtr     = trdm_dtr,
  .map     = trdm_map,
};

int trdm_register(void) {
  int result = dm_register_target(&trdm_target);
  if (result < 0) TRDM_ERRORLN("dm_register_target() failed.");
  return result;
}

void trdm_unregister(void) {
  dm_unregister_target(&trdm_target);
}

/*
setup:
	dd if=/dev/zero of=/tmp/trdm bs=512 count=4
	losetup /dev/loop1 /tmp/trdm
insmod:
	insmod trdm.ko
	echo "0 1 trdm /dev/loop1 1 3" | dmsetup create tr
rmmod:
	dmsetup remove tr
	rmmod trdm
test:
	yes A | tr -d \\n | dd of=/dev/mapper/tr; hexdump -C /tmp/trdm; hexdump -C /dev/mapper/tr
*/
