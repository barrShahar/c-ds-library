#!/bin/bash

DEST="$(dirname "$0")/headers"
mkdir -p "$DEST"

find "$(dirname "$0")" -name "*.h" | while read -r header; do
    cp "$header" "$DEST/"
done

echo "Headers copied to $DEST"
