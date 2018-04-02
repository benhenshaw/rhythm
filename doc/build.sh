NAME="doc"

echo "<style>" > $NAME.html
cat style.css >> $NAME.html
echo "</style>" >> $NAME.html

hoedown         \
--autolink      \
--strikethrough \
--underline     \
--highlight     \
--tables        \
--hard-wrap     \
$NAME.md >> $NAME.html

open $NAME.html
