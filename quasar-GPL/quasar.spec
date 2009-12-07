Version: 1.4.7_GPL
License: GPL
Name: quasar
Release: 1
Vendor: Linux Canada Inc.
URL: http://www.linuxcanada.com
Buildroot: %{_tmppath}/%{name}-%{version}-root
Source: %{name}-%{version}.tgz

Summary: Accounting software
Group: Applications/Productivity
%description
There is no package without a qualifier so this is not built.

%package server
Summary: Accounting software
Group: Applications/Productivity
Provides: quasar-server
Requires: tcl xinetd
%description server
Quasar is a business accounting package for Linux and Windows.  This
package installs the server that Quasar client connect to.  The
company databases reside on the server and are shared by all the clients.

%package client
Summary: Accounting software
Group: Applications/Productivity
Provides: quasar
Requires: tcl
%description client
Quasar is a business accounting package for Linux and Windows.  This
package installs the GUI client part of Quasar.


%prep
%setup -q

%build
./configure
make all

%install
rm -rf $RPM_BUILD_ROOT
make install PREFIX=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files server
%defattr (-,root,root)
%dir /opt/quasar
%dir /opt/quasar/backup
%dir /opt/quasar/bin
%dir /opt/quasar/config
%dir /opt/quasar/data
%dir /opt/quasar/data/companies
%dir /opt/quasar/databases
%dir /opt/quasar/drivers
%dir /opt/quasar/setup
/opt/quasar/Readme.txt
/opt/quasar/Release_143.txt
/opt/quasar/Release_144.txt
/opt/quasar/Release_145.txt
/opt/quasar/Release_146.txt
/opt/quasar/Release_147.txt
/opt/quasar/bin/quasar_setup
/opt/quasar/bin/quasard
/opt/quasar/bin/quasar_clientd
/opt/quasar/bin/quasar_import
/opt/quasar/bin/quasar_report
/opt/quasar/config/server.cfg
/opt/quasar/config/firebird.cfg
/opt/quasar/config/postgresql.cfg
/opt/quasar/data/cheque_fmts
/opt/quasar/data/handheld
/opt/quasar/data/models
/opt/quasar/data/reports
/opt/quasar/data/screens
/opt/quasar/data/shelf_labels
/opt/quasar/drivers/libfirebird_driver.so
/opt/quasar/drivers/libpostgresql_driver.so
/opt/quasar/import
/opt/quasar/setup/quasar_setup.xpm
/etc/xinetd.d/quasar

%files client
%defattr (-,root,root)
%dir /opt/quasar
%dir /opt/quasar/bin
%dir /opt/quasar/config
%dir /opt/quasar/locales
%dir /opt/quasar/locales/en_CA
%dir /opt/quasar/locales/en_CA/help
%dir /opt/quasar/locales/en_CA/help/images
%dir /opt/quasar/setup
/opt/quasar/Readme.txt
/opt/quasar/Release_143.txt
/opt/quasar/Release_144.txt
/opt/quasar/Release_145.txt
/opt/quasar/Release_146.txt
/opt/quasar/Release_147.txt
/opt/quasar/bin/quasar
/opt/quasar/bin/quasar_setup
/opt/quasar/config/client.cfg
/opt/quasar/locales/en_CA/messages.qm
/opt/quasar/locales/en_CA/messages.ts
/opt/quasar/locales/en_CA/help/*.html
/opt/quasar/locales/en_CA/help/images/*.png
/opt/quasar/setup/quasar_client.xpm
/opt/quasar/setup/quasar_setup.xpm


%post server
if [ "$1" = 1 ]; then

    # Create quasar group/user
    groupadd quasar >/dev/null 2>/dev/null
    useradd -g quasar -d /opt/quasar -s /bin/bash -c "Quasar" quasar 2>/dev/null

    # Create log file directory
    if [ ! -e /var/log/quasar ]; then
	mkdir /var/log/quasar
    fi
    chown -R quasar /var/log/quasar
    chgrp -R quasar /var/log/quasar

    # Set file ownership and permissions
    chgrp -R quasar /opt/quasar/config
    chgrp -R quasar /opt/quasar/data

    # Restart xinetd so it find new quasar entry
    /etc/init.d/xinetd restart
    /sbin/chkconfig xinetd on
fi
exit 0

%post client
if [ "$1" = 1 ]; then
    # Create quasar group/user
    groupadd quasar >/dev/null 2>/dev/null
    useradd -g quasar -d /opt/quasar -s /bin/bash -c "Quasar" quasar 2>/dev/null

    # Set file ownership and permissions
    chgrp -R quasar /opt/quasar/config
fi
exit 0

%postun server
if [ "$1" = 0 ]; then
    # Restart xinetd so it find quasar is gone
    /etc/init.d/xinetd restart

    # Kill any running servers left over
    killall -q quasard
    killall -q quasar_posd
    killall -q quasar_clientd
fi
exit 0


