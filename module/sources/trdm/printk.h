#ifndef TRDM_PRINTK_H
#define TRDM_PRINTK_H

#include <linux/printk.h>

#ifdef pr_fmt
#undef pr_fmt
#endif

#define TRLF "\n"
#define TRDM_STRINGIFY_HELPER(X) #X
#define TRDM_STRINGIFY(X) TRDM_STRINGIFY_HELPER(X)

#define TRDM_SUCCESS 0 // TODO: Move

// NOTE: pr_fmt() can be defined before the #include <linux/printk.h>.
// (Voir la définition de pr_fmt() dans linux/include/linux/printk.h)
#define pr_fmt(fmt)  \
  KBUILD_MODNAME ":" \
  __FILE_NAME__ ":"  \
  TRDM_STRINGIFY(__LINE__) ": " fmt

// __FILE_NAME__ is not standard.
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html
#define TRDM_PRINTK(level, fmt, ...)  \
  printk(                             \
    level                             \
    KBUILD_MODNAME ":"                \
    __FILE_NAME__ ":"                 \
    "%s:" /* __func__ */              \
    TRDM_STRINGIFY(__LINE__) ": " fmt \
    , __func__                        \
    , ##__VA_ARGS__                   \
  )

// Print an emergency-level message.
#define TRDM_EMERG(fmt, ...) TRDM_PRINTK(KERN_EMERG, fmt, ##__VA_ARGS__)
#define TRDM_EMERGLN(fmt, ...) TRDM_EMERG(fmt TRLF, ##__VA_ARGS__)

// Print an alert-level message.
#define TRDM_ALERT(fmt, ...) TRDM_PRINTK(KERN_ALERT, fmt, ##__VA_ARGS__)
#define TRDM_ALERTLN(fmt, ...) TRDM_ALERT(fmt TRLF, ##__VA_ARGS__)

// Print a critical-level message.
#define TRDM_CRIT(fmt, ...) TRDM_PRINTK(KERN_CRIT, fmt, ##__VA_ARGS__)
#define TRDM_CRITLN(fmt, ...) TRDM_CRIT(fmt TRLF, ##__VA_ARGS__)

// Print an error-level message.
#define TRDM_ERROR(fmt, ...) TRDM_PRINTK(KERN_ERR, fmt, ##__VA_ARGS__)
#define TRDM_ERRORLN(fmt, ...) TRDM_ERROR(fmt TRLF, ##__VA_ARGS__)

// Print a warning-level message.
#define TRDM_WARN(fmt, ...) TRDM_PRINTK(KERN_WARNING, fmt, ##__VA_ARGS__)
#define TRDM_WARNLN(fmt, ...) TRDM_WARN(fmt TRLF, ##__VA_ARGS__)

// Print a notice-level message.
#define TRDM_NOTICE(fmt, ...) TRDM_PRINTK(KERN_NOTICE, fmt, ##__VA_ARGS__)
#define TRDM_NOTICELN(fmt, ...) TRDM_NOTICE(fmt TRLF, ##__VA_ARGS__)

// Print an info-level message.
#define TRDM_INFO(fmt, ...) TRDM_PRINTK(KERN_INFO, fmt, ##__VA_ARGS__)
#define TRDM_INFOLN(fmt, ...) TRDM_INFO(fmt TRLF, ##__VA_ARGS__)

// Continues a previous log message (without '\n') in the same line.
#define TRDM_CONT(fmt, ...) printk(KERN_CONT fmt, ##__VA_ARGS__)

#endif // TRDM_PRINTK_H
