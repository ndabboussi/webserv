#!/usr/bin/env bash

echo "Content-Type: text/html"
echo ""

UPLOAD_DIR="conf/data/test/cgi-bin/uploads"
mkdir -p "$UPLOAD_DIR"

BOUNDARY=$(echo "$CONTENT_TYPE" | sed -n 's/.*boundary=\(.*\)/\1/p')

# Lire le body brut
RAW=$(dd bs=1 count=$CONTENT_LENGTH 2>/dev/null)

# Nom et extension
FILENAME=$(echo "$RAW" | awk -v RS="--$BOUNDARY" 'NR==2 {match($0, /filename="([^"]+)"/, a); print a[1]}')
EXT="${FILENAME##*.}"

# Type MIME (optionnel)
MIME_TYPE=$(echo "$RAW" | awk -v RS="--$BOUNDARY" 'NR==2 {match($0, /Content-Type: ([^ \r\n]+)/, a); print a[1]}')

# Fichier temporaire avec bonne extension
TMPFILE=$(mktemp "$UPLOAD_DIR/uploaded_file_XXXXXX.$EXT")

# Contenu pur
FILE_CONTENT=$(echo "$RAW" | awk -v RS="--$BOUNDARY" 'NR==2 {sub(/.*\r\n\r\n/, ""); sub(/\r\n$/, ""); print}')
echo -n "$FILE_CONTENT" > "$TMPFILE"

echo "<html><body>"
echo "<h1>File received!</h1>"
echo "<p>Saved to: $TMPFILE</p>"
echo "<p>Original name: $FILENAME</p>"
echo "<p>MIME type: $MIME_TYPE</p>"
echo "</body></html>"
