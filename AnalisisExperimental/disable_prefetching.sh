# --> INTEL CPUs: enables/disables various features

# allows access to MSR registers of Intel (and compatible) processors from userspace
sudo modprobe msr

# writes the value 0xF to the MSR register 0x1A4 on all processor cores
sudo wrmsr -a 0x1A4 0xF