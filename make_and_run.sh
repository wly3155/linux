set -e
set -o pipefail

LOG="$(pwd)/build.log"
echo "=== Build started $(date) ===" > "$LOG"

export ARCH=riscv
export KBUILD_OUTPUT=kbuild_out
export CROSS_COMPILE=/mnt/sda5/project/compiler/riscv/bin/riscv64-unknown-linux-gnu-
make ARCH=riscv virt_defconfig
make ARCH=riscv virt/virt.dtb
make -j$(nproc) 2>&1 | tee -a "$LOG"

export INSTALL_MOD_PATH=/home/leo/Documents/project/buildroot/module-root
make -j$(nproc) modules_install 2>&1 | tee -a "$LOG"

# Build buildroot in a subshell
(cd ../buildroot && \
    export BR2_EXTERNAL=/home/leo/Documents/project/buildroot_external && \
    make O="/home/leo/buildroot-output" qemu_riscv64_virt_defconfig && \
    make O="/home/leo/buildroot-output" -j$(nproc) 2>&1 | tee -a "$LOG")

echo "=== Build finished $(date) ===" >> "$LOG"

echo "starting simulation..."
../qemu/qemu_out/qemu-system-riscv64 -machine virt \
        -dtb kbuild_out/arch/riscv/boot/dts/virt/virt.dtb \
        -cpu rv64 -smp 4 -m 512M -nographic -device edu \
        -kernel kbuild_out/arch/riscv/boot/Image \
        -append "root=/dev/vda ro console=ttyS0" \
        -drive file=/home/leo/buildroot-output/images/rootfs.ext2,format=raw,if=virtio \
        2>&1 | tee run.log
