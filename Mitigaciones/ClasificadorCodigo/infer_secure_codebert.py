from transformers import AutoTokenizer, AutoModelForSequenceClassification
from torch.nn.functional import softmax
import torch
from test_code import *
from termcolor import colored


# --------- Load model && tokenizer trained --------- #
model_path = "./secure-codebert"
tokenizer = AutoTokenizer.from_pretrained(model_path)
model = AutoModelForSequenceClassification.from_pretrained(model_path)


# --------- Tokenize && predict --------- #
inputs = tokenizer(CODE_2, return_tensors="pt", truncation=True, padding="max_length", max_length=256) # max_length = max code characters

with torch.no_grad():
    outputs = model(**inputs)
probs = softmax(outputs.logits, dim=1)


# --------- Results --------- # 
print(f"Probability of being {colored("SECURE", "green")}:   {probs[0][1].item():.2f}")
print(f"Probability of being {colored("INSECURE", "red")}: {probs[0][0].item():.2f}")


# --> un mismo main: tecnica de ofuscacion
# --> fragmentas en funciones --> cada funcion y pasarla