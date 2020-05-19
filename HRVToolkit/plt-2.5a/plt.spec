Summary: A command-line utility for creating X11 and PostScript plots of data
Name: plt
Version: PLTVER
Release: PLTREL
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Applications
BuildRoot: %{_tmppath}/%{name}-root
Prefix: /usr

%description
`plt' is a command-line, scriptable plotting utility originally written for
Unix by Paul Albrecht.  `plt' can produce publication-quality 2D plots in an X
window or in PostScript from easily-produced text or binary data files.  The
package includes `pltf', which can generate function plots using `bc' and
`plt', `imageplt', which can generate plots of grey-scale images, and `lwcat',
which can generate PDF and PNG plots using `plt' or `pltf' and ImageMagick
converters.  The entire package, including the X11 driver, works under
GNU/Linux, Mac OS X, MS-Windows, or any version of Unix.  The latest version,
including a complete manual with many examples, is available at
http://www.physionet.org/physiotools/plt/.

%prep
%setup -q

%build
%ifarch x86_64
make XLIBDIR=/usr/X11R6/lib64 PSPDIR=/usr/lib64/ps
%else
make
%endif

%install
mkdir -p ${RPM_BUILD_ROOT}%{_prefix}/bin
mkdir -p ${RPM_BUILD_ROOT}%{_prefix}/lib/ps
mkdir -p ${RPM_BUILD_ROOT}%{_prefix}/share/man/man1
make "INSTALL_PREFIX=${RPM_BUILD_ROOT}%{_prefix}" "PREFIX=%{_prefix}" "MAN1DIR=${RPM_BUILD_ROOT}%{_prefix}/share/man/manl" install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%{_bindir}/ftable
%{_bindir}/gvcat
%{_bindir}/imageplt
%{_bindir}/lwcat
%{_bindir}/lwplt
%{_bindir}/makeplthead
%{_bindir}/plt
%{_bindir}/pltf
%{_bindir}/pltpng
%{_bindir}/xplt
%{_bindir}/xpltwin
%{_prefix}/lib/ps/plt.pro
%{_prefix}/share/man/manl/imageplt.1.gz
%{_prefix}/share/man/manl/lwcat.1.gz
%{_prefix}/share/man/manl/plt.1.gz
%{_prefix}/share/man/manl/pltf.1.gz
%defattr(-,root,root)

%changelog
* Wed Apr 27 2005 George B Moody <george@mit.edu>
- updated spec to work with updated Makefiles
* Thu May 01 2003 George B Moody <george@mit.edu>
- added lwcat man page, updated doc/book.tex
* Wed Feb 26 2003 George B Moody <george@mit.edu>
- added imageplt source, example, and man page
- avoid installing man pages in mandir to avoid conflict with wfdb-doc
* Thu Oct 17 2002 George B Moody <george@mit.edu>
- Updated summary and package description, added missing files
* Thu Oct 17 2002 Isaac C Henry <ihenry@physionet.org>
- fixed install command to use temporary rpm build root
- changed '%files' listings to use rpm's path variables
* Fri Oct 1 2002 George B Moody <george@mit.edu>
- 'make rpms' now works
* Mon Apr 1 2002 Isaac C Henry <ihenry@physionet.org>
- Initial rpm build
