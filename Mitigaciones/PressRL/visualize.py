from env import PressRL
from stable_baselines3 import PPO
import matplotlib.pyplot as plt
from tabulate import tabulate
import numpy as np



# --------- Values --------- #
TEST_DATASET = "test_latency.txt"
env = PressRL(TEST_DATASET)
model = PPO.load("PressRL_PPO")




# --------- Algorithm Prediction by Step --------- #
def prediction_by_step(model, env):
    obs, _ = env.reset()
    done = False
    steps = 0
    max_steps = 800

    rewards = []
    latencies = []
    actions = []

    while steps < max_steps:
        action, _ = model.predict(obs)
        obs, reward, done, _, _ = env.step(action)
        
        rewards.append(reward)
        latencies.append(obs[1])
        actions.append(action)

        steps += 1

        if done:
            obs, _ = env.reset()
            done = False
    
    return rewards, latencies, actions


# --------- Performance values --------- #
def evaluate_policy_performance(actions, latencies):
    correct_accesses = waits_when_unsafe = wrong_accesses = waits_when_safe = 0
    threshold = min(latencies) + 20

    for action, latency in zip(actions, latencies):
        row_closed = latency > threshold

        if action == 1: # Access
            if not row_closed:
                correct_accesses += 1
            else:
                wrong_accesses += 1
        else: # Wait
            if not row_closed:
                waits_when_safe += 1
            else:
                waits_when_unsafe += 1

    total = len(actions)
    total_accesses = correct_accesses + wrong_accesses

    accuracy = (correct_accesses + waits_when_unsafe) / total if total else 0
    precision = correct_accesses / total_accesses if total_accesses else 0
    recall = correct_accesses / (correct_accesses + waits_when_safe) if (correct_accesses + waits_when_safe) else 0
    f1 = 2 * (precision * recall) / (precision + recall) if (precision + recall) else 0
    safe_access_rate = correct_accesses / total if total else 0
    bad_access_rate = wrong_accesses / total if total else 0

    return accuracy, precision, recall, f1, safe_access_rate, bad_access_rate, total, total_accesses, correct_accesses, wrong_accesses, waits_when_safe, waits_when_unsafe


# --------- Show table performance values --------- #
def show_performance_values(accuracy, precision, recall, f1, safe_access_rate, bad_access_rate, total, total_accesses, correct_accesses, wrong_accesses, waits_when_safe, waits_when_unsafe):
    performance_values = [
        ["Total steps", total],
        ["Total accesses", total_accesses],
        ["Correct accesses", correct_accesses],
        ["Wrong accesses", wrong_accesses],
        ["Waits when safe (missed opps)", waits_when_safe],
        ["Waits when unsafe (correctly avoided)", waits_when_unsafe],
        ["Accuracy", f"{accuracy:.2f}"],
        ["Precision", f"{precision:.2f}"],
        ["Recall", f"{recall:.2f}"],
        ["F1-score", f"{f1:.2f}"],
        ["Safe access rate", f"{safe_access_rate:.2f}"],
        ["Bad access rate", f"{bad_access_rate:.2f}"]
    ]

    print("\nPRESSRL POLICY PERFORMANCE\n")
    print(tabulate(performance_values, headers=["Metric", "Value"], tablefmt="github"))



# --------- RL-Specific Evaluation Metrics --------- #
def evaluate_rl_metrics(rewards, actions, latencies):
    cumulative_reward = sum(rewards)
    avg_reward = cumulative_reward / len(rewards) if rewards else 0

    access_ratio = sum(actions) / len(actions) if actions else 0
    wait_ratio = 1 - access_ratio

    threshold = min(latencies) + 20
    correct_decisions = 0

    for a, l in zip(actions, latencies):
        row_closed = l > threshold
        if (a == 1 and not row_closed) or (a == 0 and row_closed):
            correct_decisions += 1

    decision_accuracy = correct_decisions / len(actions) if actions else 0

    return cumulative_reward, avg_reward, access_ratio, wait_ratio, decision_accuracy



# --------- Show RL metrics in table --------- #
def show_rl_metrics(cumulative_reward, avg_reward, access_ratio, wait_ratio, decision_accuracy):
    rl_values = [
        ["Cumulative Reward", f"{cumulative_reward:.2f}"],
        ["Average Reward per Step", f"{avg_reward:.3f}"],
        ["Access Ratio", f"{access_ratio:.2f}"],
        ["Wait Ratio", f"{wait_ratio:.2f}"],
        ["Decision Accuracy", f"{decision_accuracy:.2f}"]
    ]

    print("\nPRESSRL RL-SPECIFIC METRICS\n")
    print(tabulate(rl_values, headers=["Metric", "Value"], tablefmt="github"))



# --------- Plot rewards by step --------- #
def plot_rewards(rewards):
    plt.plot(rewards)
    plt.title("Rewards by step")
    plt.xlabel("Step")
    plt.ylabel("Reward")
    plt.grid()
    plt.show()


# --------- Plot rewards-latency-actions by step --------- #
def plot_rewards_latencies_actions(rewards, latencies, actions):
    plt.figure(figsize=(12, 6))

    plt.subplot(3, 1, 1)
    plt.plot(rewards, label="Reward")
    plt.legend()

    plt.subplot(3, 1, 2)
    plt.plot(latencies, label="Latency (ns)", color="orange")
    plt.legend()

    plt.subplot(3, 1, 3)
    plt.plot(actions, label="Action (0=wait, 1=access)", color="green")
    plt.legend()

    plt.tight_layout()
    plt.show()


# --------- Plot Cumulative Accuracy Over Time --------- #
def plot_cumulative_accuracy(actions, latencies):
    threshold = min(latencies) + 20
    correct = []

    for a, l in zip(actions, latencies):
        row_closed = l > threshold
        is_correct = (a == 1 and not row_closed) or (a == 0 and row_closed)
        correct.append(1 if is_correct else 0)

    cumulative_accuracy = np.cumsum(correct) / (np.arange(len(correct)) + 1)

    plt.plot(cumulative_accuracy)
    plt.title("Cumulative Accuracy over Steps")
    plt.xlabel("Step")
    plt.ylabel("Cumulative Accuracy")
    plt.grid()
    plt.show()


# --------- Plot Distribution of Rewards and Actions --------- #
def plot_distributions(rewards, actions):
    fig, axs = plt.subplots(1, 2, figsize=(12, 4))

    axs[0].hist(rewards, bins=20, color='steelblue')
    axs[0].set_title("Reward Distribution")
    axs[0].set_xlabel("Reward")
    axs[0].set_ylabel("Frequency")

    axs[1].bar(["Wait (0)", "Access (1)"], [actions.count(0), actions.count(1)], color='green')
    axs[1].set_title("Action Counts")
    axs[1].set_ylabel("Frequency")

    plt.tight_layout()
    plt.show()



# --------- Evaluate by Chunks --------- #
def evaluate_by_chunks(actions, latencies, rewards, chunk_size=200):
    num_chunks = len(actions) // chunk_size
    stats = []

    for i in range(num_chunks):
        start = i * chunk_size
        end = start + chunk_size

        acc, prec, rec, f1, safe, bad, *_ = evaluate_policy_performance(actions[start:end], latencies[start:end])
        cum_r, avg_r, acc_ratio, wait_ratio, dec_acc = evaluate_rl_metrics(rewards[start:end], actions[start:end], latencies[start:end])

        stats.append([
            i+1, f"{avg_r:.2f}", f"{dec_acc:.2f}", f"{prec:.2f}", f"{rec:.2f}", f"{f1:.2f}", f"{safe:.2f}", f"{bad:.2f}"
        ])

    headers = ["Chunk", "AvgReward", "DecisionAcc", "Precision", "Recall", "F1", "SafeAccess", "BadAccess"]
    print("\nPERFORMANCE BY CHUNKS\n")
    print(tabulate(stats, headers=headers, tablefmt="github"))




if __name__ == "__main__":
    rewards, latencies, actions = prediction_by_step(model, env)
    # plot_rewards(rewards)
    plot_rewards_latencies_actions(rewards, latencies, actions)

    # Metrics
    accuracy, precision, recall, f1, safe_access_rate, bad_access_rate, total, total_accesses, correct_accesses, wrong_accesses, waits_when_safe, waits_when_unsafe = evaluate_policy_performance(actions, latencies)
    show_performance_values(accuracy, precision, recall, f1, safe_access_rate, bad_access_rate, total, total_accesses, correct_accesses, wrong_accesses, waits_when_safe, waits_when_unsafe)

    cumulative_reward, avg_reward, access_ratio, wait_ratio, decision_accuracy = evaluate_rl_metrics(rewards, actions, latencies)
    show_rl_metrics(cumulative_reward, avg_reward, access_ratio, wait_ratio, decision_accuracy)

    plot_cumulative_accuracy(actions, latencies)
    plot_distributions(rewards, actions)

    # Evaluate by chunks
    evaluate_by_chunks(actions, latencies, rewards)
    
