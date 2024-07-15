
FROM ubuntu:20.04

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt install git -y
RUN apt install gcc -y
RUN apt install cpio -y
RUN apt install file -y
RUN apt install size -y
RUN apt install bzip2 -y
RUN apt install cppcheck -y
RUN apt install doxygen -y
RUN rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

RUN chmod +x ./scripts/*.sh
RUN chmod +x ./scripts/*.py

CMD ["bash"]
