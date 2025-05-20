
from datasets import load_dataset
from transformers import AutoTokenizer, AutoModelForSequenceClassification, Trainer, TrainingArguments
from transformers import DataCollatorWithPadding
from sklearn.metrics import accuracy_score, precision_recall_fscore_support
import numpy as np


def tokenize(example, tokenizer):
    return tokenizer(example["code"], truncation=True, padding="max_length", max_length=256)


def compute_metrics(eval_pred):
    logits, labels = eval_pred
    preds = np.argmax(logits, axis=1)
    precision, recall, f1, _ = precision_recall_fscore_support(labels, preds, average='binary')
    acc = accuracy_score(labels, preds)
    return {"accuracy": acc, "f1": f1, "precision": precision, "recall": recall}


# --------- Load dataset --------- #
dataset = load_dataset("json", data_files="code_dataset.jsonl", split="train")


# --------- Tokenizer --------- #
tokenizer = AutoTokenizer.from_pretrained("microsoft/codebert-base")
tokenized = dataset.map(lambda x: tokenize(x, tokenizer))


# --------- Model --------- #
model = AutoModelForSequenceClassification.from_pretrained("microsoft/codebert-base", num_labels=2)


# --------- Training --------- #
training_args = TrainingArguments(
    output_dir="./HammerBERT",
    num_train_epochs=10,
    per_device_train_batch_size=16,
    save_strategy="epoch",
    logging_dir="./logs"
)

# --------- Trainer --------- #
trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=tokenized,
    processing_class=tokenizer,
    data_collator=DataCollatorWithPadding(tokenizer),
    compute_metrics=compute_metrics
)

trainer.train()
trainer.save_model("./HammerBERT")  
tokenizer.save_pretrained("./HammerBERT")  

