FROM fedora:latest
RUN dnf install -y rpmdevtools qt6-qtbase-devel qt6-qttools-devel cmake && dnf clean all
COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
