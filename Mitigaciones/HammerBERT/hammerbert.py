from transformers import AutoTokenizer, AutoModelForSequenceClassification
from torch.nn.functional import softmax
import torch
from termcolor import colored
from test_codes import *


# --------- Values --------- #
SECURE = 1
INSECURE = 0
THRESHOLD = 0.85
CODES = [CODE_1, CODE_2, CODE_3, CODE_4, CODE_5, CODE_6, CODE_7, CODE_8, CODE_9, CODE_10,
         CODE_11, CODE_12, CODE_13, CODE_14, CODE_15, CODE_16, CODE_17, CODE_18, CODE_19,
         CODE_20, CODE_21, CODE_22, CODE_23, CODE_24, CODE_25]


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
    print("\n" + str(code))
    print("---------------------------------------")
    print(f"Probability of being {colored("SECURE", "green")}: {prob_secure}")
    print(f"Probability of being {colored("INSECURE", "red")}: {prob_insecure}")
    print("---------------------------------------")

    if float(prob_secure) >= THRESHOLD:
        print(f"↳ Code classified as {colored("SECURE", "green")}: the code does not contain patterns associated with hammering attacks.")

    elif float(prob_insecure) >= THRESHOLD:
        print(f"↳ Code classified as {colored("INSECURE", "red")}: the code contains patterns similar to those used in Rowhammer attacks.")

    else:
        print(f"↳ Code classified as {colored("NEUTRAL", "yellow")}: the model has not detected patterns that are clear enough to classify with confidence. Please review the code.")



# --------- Tests --------- # 
code1_prob_secure, code1_prob_insecure = classify(CODE_1)
show_results("CODE_1", code1_prob_secure, code1_prob_insecure)

code2_prob_secure, code2_prob_insecure = classify(CODE_2)
show_results("CODE_2", code2_prob_secure, code2_prob_insecure)

code3_prob_secure, code3_prob_insecure = classify(CODE_5)
show_results("CODE_12", code3_prob_secure, code3_prob_insecure)

code4_prob_secure, code4_prob_insecure = classify(CODE_4)
show_results("CODE_13", code4_prob_secure, code4_prob_insecure)

code7_prob_secure, code7_prob_insecure = classify(CODE_7)
show_results("CODE_15", code7_prob_secure, code7_prob_insecure)



# --------- Test all Codes --------- #
for i, code in enumerate(CODES, start=1):
    code_prob_secure, code_prob_insecure = classify(code)
    show_results(f"CODE_{i}", code_prob_secure, code_prob_insecure)