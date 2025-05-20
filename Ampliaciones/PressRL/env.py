import gymnasium as gym
from gymnasium import spaces
import numpy as np


class PresshammerAttackEnv(gym.Env):

    def __init__(self, latency_file):
        super().__init__()
        self.latencies = self._load_latencies(latency_file)
        self.max_steps = len(self.latencies)
        self.current_step = 0
        self.row_closed = False
        self.observation_space = spaces.Box(low=0, high=1000, shape=(2,), dtype=np.float32)
        self.action_space = spaces.Discrete(2)  # 0 = wait | 1 = access


    def _load_latencies(self, file_path):
        latencies = []

        with open(file_path, "r") as file:
            for line in file:
                values = line.strip().split()
                latencies.extend([int(latency) for latency in values[2:] if int(latency) > 0])
        
        return latencies
    

    def reset(self, seed = None, options = None):
        # self.current_step = 0
        self.row_closed = False
        latency = self.latencies[self.current_step]
        obs = np.array([self.current_step, latency], dtype=np.float32)

        return obs, {}
    

    def step(self, action):
        latency = self.latencies[self.current_step]
        self.row_closed = latency > (min(self.latencies) + 20)  # row miss
        obs = np.array([self.current_step, latency], dtype=np.float32)

        reward = 0
        if action == 1:
            if not self.row_closed:
                reward += 1 # success 
            else:
                reward -= 5 # fail  
        else:
            reward -= 0.1 # wait
        
        self.current_step += 1
        done = self.row_closed or self.current_step >= self.max_steps

        return obs, reward, done, False, {}
