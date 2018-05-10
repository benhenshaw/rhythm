pandoc \
--pdf-engine=xelatex \
--number-sections \
-Vsecnumdepth=3 \
-Vmargin-left=4cm \
-Vmargin-right=4cm \
-Vauthor="Benedict Henshaw" \
-Vdate="\today" \
-Vtitle="Implementation of an Engine for a One-Button Game" \
--from markdown+smart \
final.md -o final.pdf \
&& \
open final.pdf
