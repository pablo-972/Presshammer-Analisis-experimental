# Hardening DRAM Against Presshammer: An Experimental Study with Adaptive and AI-Based Mitigations

This repository contains the full implementation and experimental framework of **Presshammer**, a patterned DRAM disturbance attack that combines **deterministic row activation cadence** with **prolonged row exposure** to challenge classical RowHammer defenses.  
It also includes **adaptive mitigation strategies**, such as **AQRR** (Adaptive Quarantine Refreshing Row), and **AI-driven countermeasures**: **HammerBERT** (semantic-level hammer-prone code detection) and **PressRL** (reinforcement learning–based access pattern optimization).

## Features
- **Presshammer Experimental Attack**  
  - Deterministic cadence + sustained row reads to explore DRAM fault boundaries that cause bit flips.
  - Timing measurement in nanoseconds and CPU cycles.
  - Configurable number of victim rows and offsets.
  
- **Adaptive Mitigation: AQRR**  
  - Dynamically adjusts refresh strategies based on row access patterns.
  - Targets timing and access queue behavior for improved resilience.
  
- **AI-based Defenses**  
  - **HammerBERT** – CodeBERT-based transformer classifier that detects hammerstyle access patterns through code semantics.
  - **PressRL** – Reinforcement learning model to o learn to maximize row exposure while minimizing the risk of triggering disturbancebased errors, effectively balancing performance and safety.

---

## Installation & Execution

### 1. Experimental Attack (Presshammer)

  1. **Mount Huge Superpage**
     
    ```bash
    cd Analisis Experimental/cpp/setup
    sudo ./mount_huge_page.sh
    ```
  
  If it outputs `1`, the mount was successful.

2. **Build the Binary**
    
    ```bash
    cd Analisis Experimental/cpp
    make
    ```
    
3. **Run the Experiment**
    
    ```bash
    sudo ./analisis_experimental [OPTIONS]
    ```
    
    **Options:**
    
    - `-v` → Measures access time (nanoseconds and CPU cycles) for two random DRAM rows.
        
    - `-n <num_rows>` → Launches the attack against the specified number of rows starting from a fixed offset.
        

---

### 2. AI-Based Mitigations

#### HammerBERT – Code-Level RowHammer Detection

1. Install dependencies:
    
    ```bash
    pip install -r requirements.txt
    ```
    
2. Train the model:
    
    ```bash
    python train_hammerbert.py
    ```
    
    Model will be saved in the `HammerBERT` directory.
    
3. Classify test code samples:
    
    ```bash
    python hammer.py
    ```
    
4. View performance metrics & plots:
    
    ```bash
    python metrics.py
    ```
    

#### PressRL – Reinforcement Learning Access Pattern Optimization

1. Install dependencies:
    
    ```bash
    pip install -r requirements.txt
    ```
    
2. Prepare dataset from attack timing logs:
    
    ```bash
    python split_dataset.py
    ```
    
    This splits data into 80% training / 20% testing.
    
3. Train the RL model:
    
    ```bash
    python train.py
    ```
    
    Model is saved as `PressRL_PPO.zip`.
    
4. Visualize and evaluate:
    
    ```bash
    python visualize.py
    ```

---

### 3. Disclaimer

This repository is provided **for research and educational purposes only**.  
The authors are not responsible for any misuse or damage resulting from the use of this code.
