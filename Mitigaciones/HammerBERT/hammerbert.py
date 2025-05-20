from transformers import AutoTokenizer, AutoModelForSequenceClassification
from torch.nn.functional import softmax
import torch
from termcolor import colored
from test_codes import *


# --------- Values --------- #
SECURE = 1
INSECURE = 0
THRESHOLD = 0.7

# --------- Load model && tokenizer trained --------- #
MODEL_PATH = "./HammerBERT"
MODEL = AutoModelForSequenceClassification.from_pretrained(MODEL_PATH)
TOKENIZER = AutoTokenizer.from_pretrained(MODEL_PATH)


# --------- Tokenize && classifier --------- #
def classify(code):
    inputs = TOKENIZER(code, return_tensors="pt", truncation=True, padding="max_length", max_length=512) # max_length = max code characters

    with torch.no_grad():
        outputs = MODEL(**inputs)

    probs = softmax(outputs.logits, dim=1)
    prob_secure = f"{probs[0][SECURE].item():.2f}"
    prob_insecure = f"{probs[0][INSECURE].item():.2f}"

    return prob_secure, prob_insecure


# --------- Show results --------- #
def show_results(code, prob_secure, prob_insecure):
    print("\n" + code)
    print("---------------------------------------")
    print(f"Probability of being {colored("SECURE", "green")}: {prob_secure}")
    print(f"Probability of being {colored("INSECURE", "red")}: {prob_insecure}")

    if float(prob_secure) >= THRESHOLD:
        print(f"{colored("SECURE", "green")} CODE")
    
    elif float(prob_insecure) >= THRESHOLD:
        print(f"{colored("INSECURE", "red")} CODE")

    else:
        print("UNCLASSIFIABLE CODE (NEUTRAL)")



# --------- Tests --------- # 
code1_prob_secure, code1_prob_insecure = classify(CODE_1)
show_results("CODE_1", code1_prob_secure, code1_prob_insecure)

code2_prob_secure, code2_prob_insecure = classify(CODE_2)
show_results("CODE_2", code2_prob_secure, code2_prob_insecure)

code3_prob_secure, code3_prob_insecure = classify(CODE_5)
show_results("CODE_5", code3_prob_secure, code3_prob_insecure)

code4_prob_secure, code4_prob_insecure = classify(CODE_4)
show_results("CODE_4", code4_prob_secure, code4_prob_insecure)

code7_prob_secure, code7_prob_insecure = classify(CODE_7)
show_results("CODE_7", code7_prob_secure, code7_prob_insecure)
