#!/bin/bash

set -euo pipefail

DOCKER_IMAGE="devkitpro/devkitarm:latest"

if [ $# -ne 2 ]; then
	echo "Usage: $(basename "$0") <theme file> <output directory>"
	exit 1
fi

INPUT_FILE="$1"
OUTPUT_DIR="$2"

if [ -d "$INPUT_FILE" ]; then
	echo "$INPUT_FILE is a file but a file is expected. Exiting."
	exit 1
fi

INPUT_DIR=$(cd "$(dirname "$INPUT_FILE")" && pwd)
THEME_NAME=$(basename "$INPUT_FILE" .ibt)
mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR=$(cd "$OUTPUT_DIR" && pwd)
OUTPUT_FILE="$OUTPUT_DIR/$(basename "$INPUT_FILE")"
cp "$INPUT_FILE" "$OUTPUT_FILE"

echo "--atlas -f rgba8888 -z auto" > "$OUTPUT_DIR/$THEME_NAME.t3s"

grep -iv '^#' "$OUTPUT_FILE" | grep -i bitmap | awk '{print $2;}' | while read bitmap; do
	if [ ! -f "$INPUT_DIR/$bitmap.png" ]; then
		echo "Cannot find file $INPUT_DIR/$bitmap.png. Exiting."
		exit 1
	fi
	cp "$INPUT_DIR/$bitmap.png" "$OUTPUT_DIR"/
	echo "$bitmap.png" >> "$OUTPUT_DIR/$THEME_NAME.t3s"
done

if command -v docker &> /dev/null; then
	DOCKER=docker
	USER_OPTS="-u $(id -u):$(id -g)"
elif command -v podman &> /dev/null; then
	DOCKER=podman
	USER_OPTS="--userns=keep-id"
else
	echo "Neither docker nor podman has been found. Exiting."
	exit 1
fi

"$DOCKER" run --rm $USER_OPTS -v "$OUTPUT_DIR:/work" -w "/work" "$DOCKER_IMAGE" \
	tex3ds -i "$THEME_NAME.t3s" -H "$THEME_NAME.h" -o "$THEME_NAME.t3x"

while read file; do
	[ -f "$OUTPUT_DIR/$file" ] && rm -f "$OUTPUT_DIR/$file"
done < "$OUTPUT_DIR/$THEME_NAME.t3s"

rm -f "$OUTPUT_DIR/$THEME_NAME.t3s"

if ! command -v ffmpeg &> /dev/null; then
	echo "ffmpeg not found. Skipping sounds"
	sed -i.bak '/^sound/Id' "$OUTPUT_FILE"
else
	for sound in $(grep -i ^sound "$OUTPUT_FILE" | awk '{print $2;}' | sort -u); do
		if [ ! -f "$INPUT_DIR/$sound.wav" ] && [ ! -f "$INPUT_DIR/$sound.mp3" ]; then
			echo "Cannot find either file $INPUT_DIR/$sound.wav or file $INPUT_DIR/$sound.mp3. Exiting."
			exit 1
		fi
		if [ -f "$INPUT_DIR/$sound.wav" ]; then
			_input="$INPUT_DIR/$sound.wav"
		else
			_input="$INPUT_DIR/$sound.mp3"
		fi
		ffmpeg -hide_banner -loglevel error -nostats -y -i "$_input" -ac 1 -ar 22050 \
			-c:a pcm_s16le -f s16le "$OUTPUT_DIR/$sound.raw"
	done
fi

echo "Theme created. Please copy the contents of $OUTPUT_DIR in sdmc:/icebreaker"
