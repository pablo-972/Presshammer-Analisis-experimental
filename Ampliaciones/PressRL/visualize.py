from env import PresshammerAttackEnv
from stable_baselines3 import PPO
import matplotlib.pyplot as plt

TEST_DATASET = "test_latency.txt"


# --------- Visualize --------- #
env = PresshammerAttackEnv(TEST_DATASET)
model = PPO.load("ppo_presshammer_attacker")
obs, _ = env.reset()
done = False
steps = 0
max_steps = 1000


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


# Show steps rewards in chart
plt.plot(rewards)
plt.title("Recompensas por paso (Ataque)")
plt.xlabel("Paso")
plt.ylabel("Recompensa")
plt.grid()
plt.show()


# Show rewards by latency in chart
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
