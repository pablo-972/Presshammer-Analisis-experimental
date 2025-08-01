from test_codes import *
from hammerbert import THRESHOLD, CODES, classify
from tabulate import tabulate
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns


# --------- Values --------- #
# List with the actual code labels (in order). Make sure this matches exactly the order of the codes in `CODES`.
REAL_LABELS = ["INSECURE", "INSECURE", "INSECURE", "INSECURE", "INSECURE", "INSECURE", "INSECURE", "INSECURE", "INSECURE", "INSECURE",
                "INSECURE", "INSECURE", "INSECURE", "INSECURE", "SECURE", "SECURE", "SECURE", "SECURE", "SECURE", 
                "SECURE", "SECURE", "SECURE", "SECURE", "SECURE", "SECURE", "SECURE", "SECURE", "SECURE",
                "SECURE", "SECURE"]  



# --------- Algorithm to Calculate Metrics --------- #
def calculate_metrics(codes, real_labels):
    tp = tn = fp = fn = classified = 0  # true positives, true negatives, false positives, false negatives, classified
    total = len(REAL_LABELS)
    results = []

    for i, (code, real_label) in enumerate(zip(codes, real_labels), start=1):
        prob_secure, prob_insecure = classify(code)
        prob_secure = float(prob_secure)
        prob_insecure = float(prob_insecure)

        # Classify fragment code using threshold
        if prob_secure >= THRESHOLD:
            pred_label = "SECURE"
        elif prob_insecure >= THRESHOLD:
            pred_label = "INSECURE"
        else:
            pred_label = "NEUTRAL"

        # Save results: real, predicted, prob_secure, prob_insecure
        results.append((f"CODE_{i}", real_label, pred_label, prob_secure, prob_insecure))

        # Count tp, tn, fp, fn, classified
        if pred_label != "NEUTRAL":
            classified += 1
        if pred_label == "SECURE":
            if real_label == "SECURE":
                tn += 1
            else:
                fn += 1
        elif pred_label == "INSECURE":
            if real_label == "INSECURE":
                tp += 1
            else:
                fp += 1

        
    # Metrics 
    accuracy = (tp + tn) / classified if classified > 0 else 0
    precision = tp / (tp + fp) if (tp + fp) > 0 else 0
    recall = tp / (tp + fn) if (tp + fn) > 0 else 0
    f1 = 2 * (precision * recall) / (precision + recall) if (precision + recall) > 0 else 0
    coverage = classified / total

    return tp, tn, fp, fn, accuracy, precision, recall, f1, coverage, total, classified, results


# --------- Performance Values Result --------- #
def show_performance_values_result(accuracy, precision, recall, f1, coverage, total, classified):
    performance_values = [
        ["Total fragments evaluated", total],
        ["Confidently classified fragments", f"{classified} ({coverage*100:.2f}%)"],
        ["Accuracy", f"{accuracy:.2f}"],
        ["Precision", f"{precision:.2f}"],
        ["Recall", f"{recall:.2f}"],
        ["F1-score", f"{f1:.2f}"]
    ]

    print("\nHAMMERBERT MODEL PERFORMANCE RESULTS\n")
    print(tabulate(performance_values, headers=["Metric", "Value"], tablefmt="github"))



# --------- Comparison between Classified Fragments --------- #
def comparison_fragments(results):
    classified_fragments = []
    for code_id, real, pred, ps, pi in results:
        classified_fragments.append([code_id, real, pred, f"{ps:.2f}", f"{pi:.2f}"])

    print("\nCOMPARISON BETWEEN CLASSIFIED FRAGMENTS\n")
    print(tabulate( classified_fragments, headers=["Fragment", "Real", "Predicted", "P(SECURE)", "P(INSECURE)"], tablefmt="github"))



# --------- Plot Metrics --------- #
def plot_model_metrics(accuracy, precision, recall, f1, coverage):
    metric_names = ["Accuracy", "Precision", "Recall", "F1-score", "Coverage"]
    metric_values = [accuracy, precision, recall, f1, coverage]

    plt.figure(figsize=(8, 5))
    plt.bar(metric_names, metric_values, color="steelblue", edgecolor="black")
    plt.ylim(0, 1)
    plt.title("Performance Values - HammerBERT")
    plt.ylabel("Value")
    plt.grid(axis="y", linestyle="--", alpha=0.7)
    plt.tight_layout()
    plt.show()


# --------- Plot Fragment Confidences --------- #
def plot_fragment_confidences(results, threshold):
    fragment_ids = [code_id for code_id, *_ in results]
    secure_probs = [ps for *_, ps, _ in results]
    insecure_probs = [pi for *_, _, pi in results]

    plt.figure(figsize=(12, 6))
    plt.plot(fragment_ids, secure_probs, label="P(SECURE)", marker='o', color="green")
    plt.plot(fragment_ids, insecure_probs, label="P(INSECURE)", marker='x', color="red")
    plt.axhline(threshold, color="gray", linestyle="--", label=f"Threshold = {threshold}")
    plt.xticks(rotation=90)
    plt.ylabel("Probability")
    plt.title("Model confidence by fragment - HammerBERT")
    plt.legend()
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()
    plt.show()


# --------- Plot Confusion Matrix --------- #
def plot_confusion_matrix(tp, fp, tn, fn):
    matrix = np.array([[tp, fn], [fp, tn]])
    labels = ["INSECURE", "SECURE"]

    plt.figure(figsize=(5, 4))
    sns.heatmap(matrix, annot=True, fmt="d", cmap="Blues", xticklabels=labels, yticklabels=labels)
    plt.xlabel("Real")
    plt.ylabel("Prediction")
    plt.title("Confusion Matrix - HammerBERT")
    plt.tight_layout()
    plt.show()



if __name__ == "__main__":
    tp, tn, fp, fn, accuracy, precision, recall, f1, coverage, total, classified, results = calculate_metrics(CODES,REAL_LABELS)
    show_performance_values_result(accuracy, precision, recall, f1, coverage, total, classified)
    comparison_fragments(results)

    # Charts
    plot_model_metrics(accuracy, precision, recall, f1, coverage)
    plot_fragment_confidences(results, THRESHOLD)
    plot_confusion_matrix(tp, fp, tn, fn)
