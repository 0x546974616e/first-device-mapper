
#let project(
  title: none,
  author: none,
  body,
) = {
  // Set document metadata.
  set document(title: title, author: author)
  set page(margin: auto)
  set par(
    justify: false,
    leading: 0.55em,
    spacing: 0.55em,
    first-line-indent: (
      amount: 1em,
      all: true,
    ),
  )

  set text(lang: "fr")
  set text(font: "New Computer Modern")
  set quote(block: true)

  set heading(numbering: "I.1.a")
  show heading: set block(above: 1.5em, below: 1em)
  show heading.where(level: 1): it => (
    pagebreak(weak: true) + it + v(-1em) + line(length: 100%)
  )

  v(1fr) + align(center, text(2em, title)) + v(3fr)
  pagebreak(weak: true)
  outline()

  set page(numbering: "1 / 1")
  counter(page).update(1)
  pagebreak(weak: true)
  body
}
