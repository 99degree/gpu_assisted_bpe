import json
import inflect

# Initialize number-to-word engine
p = inflect.engine()

# Generate 1024 entries
vocab = {}
for i in range(4096):
    word_key = p.number_to_words(i).replace("-", " ").replace(",", "").lower()
    vocab[word_key] = i

model = { "vocab" : vocab }
body = { "model" : model }



# Save to JSON (optional)
with open("tokenizer.json", "w") as f:
    json.dump(body, f, indent=2)
