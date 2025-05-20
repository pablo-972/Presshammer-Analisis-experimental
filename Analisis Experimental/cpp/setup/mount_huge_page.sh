# https://github.com/comsec-group/blacksmith/issues/2#issuecomment-971810211

# huge page
gbFile="/sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages"

# create directory
mkdir /mnt/huge

# mount huge page of 1GiB
mount -t hugetlbfs  -o pagesize=1G none /mnt/huge

# check if done correctly
su -c 'echo 1 >'$gbFile
cat $gbFile