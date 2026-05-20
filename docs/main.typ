#import "template.typ": project

#show: project.with(
  title: "TRDM documentation",
  author: "Tristan",
)

#include "chapters/setup.typ"
#include "chapters/mapper.typ"
#include "chapters/preboot.typ"

#bibliography("references.yaml", full: true)
