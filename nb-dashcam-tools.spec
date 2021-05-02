Name:           nb-dashcam-tools
Version:        %{version}
Release:        1%{?dist}
Summary:        Tools for merging clips and extracting GPS data from Nextbase dashcam video files

License:        GPLv3
URL:            https://github.com/skyhisi/nb-dashcam-tools
Source0:        https://github.com/skyhisi/nb-dashcam-tools/releases/latest/download/nb-dashcam-tools.tgz

BuildRequires:  qt5-qtbase-devel cmake
Requires:       ffmpeg qt5-qtbase qt5-qtbase-gui

%description
Tools for merging clips and extracting GPS data from Nextbase dash cam video files.


%prep
%setup -n nb-dashcam-tools


%build
%cmake 
%cmake_build


%install
rm -rf $RPM_BUILD_ROOT
%cmake_install


%files
%license LICENSE
%doc README.md
%attr(755, root, root) %{_bindir}/nb-dashcam-tools


%changelog
* Sun May 02 2021 Silas Parker <skyhisi@gmail.com>
- Initial spec file
- 
