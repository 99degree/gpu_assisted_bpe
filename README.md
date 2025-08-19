# gpu_assisted_bpe
(C)Copyright 2025 George Chan

Use my old buddy iGPU HD620 to do token matching.

## License
This project as whole is released as GPLv2. If you found those code are not having a license header then please treat them as GPLv2. I assumed people get a copy of those code notice the copyright.

## Abstract
This is to experiment offloading tokenizerBPE matching into Vulkan GPU. It does have some good result.

There is missing letter normalization and pre-tokenization, count pair and iterative merging function are missing. 

So far this code only works with plain ascii. Other letters are most likely treat as space.

## Method
Get llm tokenizer.json and transform into fixed length, then fed into gpu for threaded comparison to user context. Result coded as fix width tokens and display to user the result.

### Available algos
There are currently 2 available algos.

token_match.comp -- This is bpe core Longest matching algo, aka TableBPE
token_bsp_utf.comp -- This is utf8 -> utf32 convert algo, aka DashUTF32

Generally DashUTF32 algo is a kind of pre-requist for GPU BPE either implement in GPU or CPU. According to the nature of GPU arch, it's addressing mode is by 32bits or 64bits. Due to this reason, it is either transform utf-8 (variable length coding) into utf-32 (fixed length) by CPU and transfer to GPU process; or single pass with GPU. So to speed things up, it is best to do in GPU with long input to compensate the full trip HOST<->GPU expense.

If input string is short like few kilobyte, it is better to do in CPU. It is very simple as user input is short and user uploaded files long enough.

Once GPU is loaded with fixed sized wording as input, TableBPE is able to trim in for token matching.

### Status
Since HuggingFase's BPE is more than matching, as below, so similar trick can be applied either by TableBPE method or DashUTF32 method.
- input verification -- No available detail
- input trunking -- nil, seemed no needed
- input merging -- similar to TableBPE
- input utf normalization (NFKD/NFC/NFD) -- similar to TableBPE
- input tokenizing/matching -- on-par with TableBPE

So far this project is not ready to replace llama/HuggingFace BPE atm.

## Result
This code shows some buggy output with WSL2 dozen vulkan driver and yet to get fixed.
LLVM-pipe is running fine but slow as expected. Still yet to prove stable in real GPU. 

Testing my own build of token key library, using dozen vs llvm:

Tokens: 4095 in dictionary, each token aveage length 20 chars, total 207319 bytes. Compute Shader with 64 threads enabled.

llvm:
⏱️ Dispatch time: 558031 ms
✅ Tokenization complete.
558031/4095 = 136 ms/Tok _OR_ 0.0073 Tok/ms

dozen:
⏱️ Dispatch time: 144046 ms
✅ Tokenization complete.
144046/4095 = 35 ms/Tok _OR_ 0.0284 Tok/ms

Around 3.78 times faster in dozen.

For short message context:
The tokenizing procedure last long for about 12sec plus loading overhead around 15sec. 
A bit faster than my own test on llama.cpp with qwen llm running under Intel i5-7200u.

Output:
⏱️ Dispatch time: 12364.8 ms
✅ Tokenization complete.
🧠 Encoded token results:
[0] → TokenID: 90 Text: {
[1] → TokenID: 1 Text: "

## build

glslc token_match.comp -o token_match.spv
_OR_
glslc token_bsp_utf.comp -o token_match.spv

cmake ./
make

Done!

## How to test
Choose tokenizer.json.* file and rename to tokenizer.json
./qwen_shader -f tokenizer.json

_OR_
./qwen_shader -n 100 "the quick fox is jump over the lazy dog"

Then test app will test to tokenize the tokenizer.json file to with itself. Choose other file to test is also fine.

## Output

📁 Loaded prompt from file: "tokenizer.json"
📝 Prompt: "{
  "model": {
    "vocab": {
      "zero": 0,
      "one": 1,
      "two": 2,
      "three": 3,
      "four": 4,
      "five": 5,
      "six": 6,
      "seven": 7,
      "eight": 8,
      "nine": 9,
      "ten": 10,
...
      "ninety three": 93,
      "ninety four": 94,
      "ninety five": 95,
      "ninety six": 96,
</s>"
inputLenBytes:2277 input32.size() 2277
📚 DictionaryBuilder: Packing tokens
  [0] Token: "two thousand two hundred and two"
    Code: 2202, Length: 32
    Aligned: 116 119 111 32 116 104 111 117 115 97 110 100 32 116 119 111 32 104 117 110 100 114 101 100 32 97 110 100 32 116 119 111
  [1] Token: "two thousand two hundred and twenty seven"
    Code: 2227, Length: 41
    Aligned: 116 119 111 32 116 104 111 117 115 97 110 100 32 116 119 111 32 104 117 110 100 114 101 100 32 97 110 100 32 116 119 101 110 116 121 32 115 101 118 101 110
  [2] Token: "two thousand two hundred and twenty nine"
    Code: 2229, Length: 40
    Aligned: 116 119 111 32 116 104 111 117 115 97 110 100 32 116 119 111 32 104 117 110 100 114 101 100 32 97 110 100 32 116 119 101 110 116 121 32 110 105 110 101
  [3] Token: "two thousand two hundred and twelve"
    Code: 2212, Length: 35
    Aligned: 116 119 111 32 116 104 111 117 115 97 110 100 32 116 119 111 32 104 117 110 100 114 101 100 32 97 110 100 32 116 119 101 108 118 101
...
🔁 inputCursor = 2275 missed 1 byte
🔁 inputCursor = 2276 missed 1 byte
🔁 inputCursor = 2277 missed 1 byte
⏱️ Dispatch time: 3120.38 ms
✅ Tokenization complete.
🧠 Encoded token results:
[1] → TokenID: 1 Text: one
[2] → TokenID: 2 Text: two
[3] → TokenID: 3 Text: three
[4] → TokenID: 4 Text: four
[5] → TokenID: 5 Text: five
[6] → TokenID: 6 Text: six
[7] → TokenID: 7 Text: seven
[8] → TokenID: 8 Text: eight
[9] → TokenID: 9 Text: nine
[10] → TokenID: 10 Text: ten
[11] → TokenID: 11 Text: eleven
[12] → TokenID: 12 Text: twelve
[13] → TokenID: 13 Text: thirteen
...
