#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: ./ask.sh \"your question here\""
    echo "Or run ./bricllm for interactive mode"
    exit 1
fi

QUESTION="$1"

echo "$QUESTION" | ./bricllm | grep -A 100 "You: " | tail -n +2 | head -n 20
