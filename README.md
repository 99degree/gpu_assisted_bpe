# gpu_assisted_bpe
(C)Copyright 2025 George Chan

## Use my old buddy iGPU HD620 to do token matching.

## License
This project as whole is released as GPLv2. If you found those code are not having a license header then please treat them as GPLv2. I assumed people get a copy of those code notice the copyright.

## Idea of the code itself
This is to experiment offloading tokenizerBPE into Vulkan GPU. It does have some good result.

## Method
Get llm tokenizer.json and transform into fixed length, then fed into gpu for threaded comparison to user context. Result coded as fix width tokens and display to user the result.

## Result
This code shows some buggy output yet to get fixed. Not yet confirm if this is hardware bug or not.

