#!/usr/bin/env bash
set -e

echo "Content-Type: text/html"
echo ""

UPLOAD_DIR="../uploads"
mkdir -p "$UPLOAD_DIR"

# Extract boundary
BOUNDARY=$(echo "$CONTENT_TYPE" | sed -n 's/.*boundary=\(.*\)/\1/p')
if [ -z "$BOUNDARY" ]; then
    echo "<h1>Error: No boundary found</h1>"
    exit 1
fi

# Read raw body to a temporary file
BODY_FILE=$(mktemp /tmp/body.XXXXXX)
dd bs=1 count="$CONTENT_LENGTH" of="$BODY_FILE" 2>/dev/null

# Get the part containing the uploaded file headers
# Find where the filename is declared
HEADER_START=$(grep -abo "filename=" "$BODY_FILE" | head -n1 | cut -d: -f1)

if [ -z "$HEADER_START" ]; then
    echo "<h1>No file found in POST data</h1>"
    rm -f "$BODY_FILE"
    exit 0
fi

# Extract the filename
FILENAME=$(tail -c +$((HEADER_START+1)) "$BODY_FILE" | \
    sed -n 's/.*filename="\([^"]*\)".*/\1/p' | head -n1)
BASENAME=$(basename "$FILENAME")
EXT="${BASENAME##*.}"

# Find where file data starts: after 2 consecutive CRLF
DATA_START=$(grep -abo -m1 $'\r\n\r\n' "$BODY_FILE" | head -n1 | cut -d: -f1)
DATA_START=$((DATA_START + 4))

# Find where the boundary of this part ends
BOUNDARY_POS=$(grep -abo -- "--$BOUNDARY" "$BODY_FILE" | tail -n1 | cut -d: -f1)

# Calculate file data length
DATA_LEN=$((BOUNDARY_POS - DATA_START - 2))

# Write pure file data
OUT_FILE=$(mktemp "$UPLOAD_DIR/upload_XXXXXX.$EXT")
dd if="$BODY_FILE" of="$OUT_FILE" bs=1 skip="$DATA_START" count="$DATA_LEN" 2>/dev/null

# Clean up
rm -f "$BODY_FILE"

echo "<html><body>"
echo "<h1>File uploaded successfully!</h1>"
echo "<p>Saved to: $OUT_FILE</p>"
echo "<p>Original name: $BASENAME</p>"
echo "<p>Size: $(stat -c%s "$OUT_FILE") bytes</p>"
echo "</body></html>"
