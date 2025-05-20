from stable_baselines3 import PPO
from env import PresshammerAttackEnv

TRAIN_DATASET = "train_latency.txt"


# --------- Training --------- #
env = PresshammerAttackEnv(TRAIN_DATASET)
model = PPO("MlpPolicy", env, verbose=1, device="cpu")
model.learn(total_timesteps=10_000)
model.save("ppo_presshammer_attacker")