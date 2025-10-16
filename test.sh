#!/bin/bash

echo "Testing Bricllm with sample questions..."
echo ""

questions=(
    "How do I pay rent?"
    "Where can I find maintenance requests?"
    "Show me navigation buttons"
    "What is broken?"
)

for question in "${questions[@]}"; do
    echo "========================================"
    echo "Q: $question"
    echo "========================================"
    echo "$question
/quit" | ./bricllm 2>&1 | grep -A 5 "Bricllm:" | head -n 6
    echo ""
    sleep 1
done

echo "Test complete!"
