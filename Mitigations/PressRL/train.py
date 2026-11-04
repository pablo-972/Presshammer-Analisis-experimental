from env import PressRL
from stable_baselines3 import PPO


# --------- Training Dataset --------- #
TRAIN_DATASET = "train_latency.txt"


# --------- Training --------- #
env = PressRL(TRAIN_DATASET)
model = PPO("MlpPolicy", env, verbose=1, device="cpu")
model.learn(total_timesteps=10_000)
model.save("PressRL_PPO")