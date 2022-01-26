FROM debian:stretch-20211220-slim
RUN apt-get update 
RUN apt-get install -y gcc nasm llvm vim build-essential mtools



