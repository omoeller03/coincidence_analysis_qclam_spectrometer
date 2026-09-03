# Trajektorienrekonstruktion



## Installation

Projekt bauen:

```bash
mkdir build && cd build
cmake ..
make -j
```

## Nutzung

SIEHE IKP-WIKI

Die Analyse kann in verschiedenen Modi laufen:

- QCLAM_SINGLE: Analysiere nur die QCLAM-Daten.
- QCLAM_DAGOBERT: Analysiere QCLAM und DAGOBERT Daten und bilde am Ende Koinzidenz.
- DAGOBERT_SINGLE: TODO
- QCLAM_SINGLE_ROOT: Analysiere nur QCLAM-Daten, nutze entpackte Root-Daten als Input.


Um moeglichst wenig zu streamen und die Analyse dadurch schnell zu halten, muessen diese Modi vor dem Kompilieren festgelegt werden.
Alle kompilierten Schritte koennen beim Ausfuehren zusaetzlich aktiviert/deaktiviert werden, sodass es im QCLAM_DAGOBERT-Modus auch moeglich ist,
nur QCLAM-Daten zu analysieren.

Der Modus muss in der CMakeLists.txt eingestellt werden:

```bash
# modus QCLAM_SINGLE (kein Dagobert, DEFERA etc)
option(QCLAM_SINGLE "Build QCLAM_SINGLE" OFF)
# modus QCLAM_SINGLE (kein Dagobert, DEFERA etc)
option(QCLAM_DAGOBERT "Build QCLAM_DAGOBERT" ON)
# modus QCLAM_SINGLE_ROOT (nur QCLAM, mit Root-Input)
option(QCLAM_SINGLE_ROOT "Build QCLAM_SINGLE_ROOT" OFF)
```


## Getting started

To make it easy for you to get started with GitLab, here's a list of recommended next steps.

Already a pro? Just edit this README.md and make it your own. Want to make it easy? [Use the template at the bottom](#editing-this-readme)!

## Add your files

- [ ] [Create](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#create-a-file) or [upload](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#upload-a-file) files
- [ ] [Add files using the command line](https://docs.gitlab.com/ee/gitlab-basics/add-file.html#add-a-file-using-the-command-line) or push an existing Git repository with the following command:

```
cd existing_repo
git remote add origin http://gitlab.ikp.physik.tu-darmstadt.de/omoeller/trajektorienrekonstruktion.git
git branch -M main
git push -uf origin main
```

## Integrate with your tools

- [ ] [Set up project integrations](http://gitlab.ikp.physik.tu-darmstadt.de/omoeller/trajektorienrekonstruktion/-/settings/integrations)

## Collaborate with your team

- [ ] [Invite team members and collaborators](https://docs.gitlab.com/ee/user/project/members/)
- [ ] [Create a new merge request](https://docs.gitlab.com/ee/user/project/merge_requests/creating_merge_requests.html)
- [ ] [Automatically close issues from merge requests](https://docs.gitlab.com/ee/user/project/issues/managing_issues.html#closing-issues-automatically)
- [ ] [Enable merge request approvals](https://docs.gitlab.com/ee/user/project/merge_requests/approvals/)
- [ ] [Set auto-merge](https://docs.gitlab.com/ee/user/project/merge_requests/merge_when_pipeline_succeeds.html)

## Test and Deploy

Use the built-in continuous integration in GitLab.

- [ ] [Get started with GitLab CI/CD](https://docs.gitlab.com/ee/ci/quick_start/index.html)
- [ ] [Analyze your code for known vulnerabilities with Static Application Security Testing(SAST)](https://docs.gitlab.com/ee/user/application_security/sast/)
- [ ] [Deploy to Kubernetes, Amazon EC2, or Amazon ECS using Auto Deploy](https://docs.gitlab.com/ee/topics/autodevops/requirements.html)
- [ ] [Use pull-based deployments for improved Kubernetes management](https://docs.gitlab.com/ee/user/clusters/agent/)
- [ ] [Set up protected environments](https://docs.gitlab.com/ee/ci/environments/protected_environments.html)

***

# Editing this README

When you're ready to make this README your own, just edit this file and use the handy template below (or feel free to structure it however you want - this is just a starting point!). Thank you to [makeareadme.com](https://www.makeareadme.com/) for this template.

## Suggestions for a good README
Every project is different, so consider which of these sections apply to yours. The sections used in the template are suggestions for most open source projects. Also keep in mind that while a README can be too long and detailed, too long is better than too short. If you think your README is too long, consider utilizing another form of documentation rather than cutting out information.

## Name
Choose a self-explaining name for your project.

## Description
Let people know what your project can do specifically. Provide context and add a link to any reference visitors might be unfamiliar with. A list of Features or a Background subsection can also be added here. If there are alternatives to your project, this is a good place to list differentiating factors.

## Badges
On some READMEs, you may see small images that convey metadata, such as whether or not all the tests are passing for the project. You can use Shields to add some to your README. Many services also have instructions for adding a badge.

## Visuals
Depending on what you are making, it can be a good idea to include screenshots or even a video (you'll frequently see GIFs rather than actual videos). Tools like ttygif can help, but check out Asciinema for a more sophisticated method.

## Installation
Es muessen Root, Genfit und Go4 installiert sein. Falls noch nicht installiert (siehe auch im IKP-Wiki http://ikpweb/mediawiki/index.php/QCLAM#Installation_von_Software):
Beginne mit Root, da Go4 und Genfit es brauchen:
* ROOT:
 Root  v6-30-04 habe ich getestet. Neuere Root-Versionen wurden nicht getestet

 i) Abhaengigkeiten:

	sudo apt install -y git cmake g++ gcc binutils \
	libx11-dev libxpm-dev libxft-dev libxext-dev \  
	python3-dev libssl-dev libpcre3-dev \  
	libglu1-mesa-dev libglew-dev libftgl-dev \  
	libmysqlclient-dev libfftw3-dev \
	libgraphviz-dev libavahi-compat-libdnssd-dev \  
	libldap2-dev python3-numpy libxml2-dev \  
	libkrb5-dev libgsl-dev libqt5websockets5-dev
	
	In manchen Linux-Versionen 'sudo apt install libmysqlclient-dev libfftw3-dev' durch 'sudo apt install default-libmysqlclient-dev' ersetzt werden. 

 ii) Klone Projekt:

	git clone https://github.com/root-project/root.git
	cd root
	git checkout v6-30-04

 iii) baue Root:

	cmake ../root -DCMAKE_INSTALL_PREFIX=/home/.../root/root-install
	make -j$(nproc)
	sudo make install (falls gewuenscht)

 iv) Umgebung setzen:

	source /home/.../root/root-install/bin/thisroot.sh

* Genfit:

 i) Abhaengigkeiten:

	Root -> installiert

	sudo apt install libgtest-dev
	cd /usr/src/getest
	sudo cmake .
	sudo make
	sudo cp lib/*.a /usr/lib

 ii) Klone Projekt:

	git clone https://github.com/GenFit/GenFit.git

 iii) Bauen:

	Root sourcen
	cd GenFit
	mkdir build && cd build
	cmake .. -DBUILD_SHARED_LIBS=ON
	make
	make doc
	make tests
	sudo make install (falls gewuenscht)

	ggf. muss 'TESTING' in der CMakeLists (Ende) auskommentiert werden:
	
	INCLUDE(CTEST)
	...
	IF(DEFINED REAVE)
	...
	ENDIF
	IF(BUILD_TESTING)
	...
	ENDIF()

* Go4:

 i) Abhaengigkeiten:

	Root -> installiert

	sudo apt update && sudo install git cmake g++ qtbase5-dev qt5-qmake libqt5widgets5 

 ii) Klone Projekt (hier fuer v6.4.99):

	git clone https://github.com/gsi-ee/go4.git

 iii) Bauen:

	Root sourcen
	
	cd go4/build 
	cmake ../ \
 	-DCMAKE_INSTALL_PREFIX=$HOME/<Pfad zu go4>/go4-install \
 	-DCMAKE_CXX_STANDARD=17
	make -j
	sudo make install

 iv) Umgebung setzen:

	source /home/.../go4-install/go4login

Wenn Root, Genfit und Go4 eingerichtet sind, kann dieses Projekt gebaut werden.
Das Projekt ist aufgeteilt in die eigentliche Analyse und die Uebergabe dieser an Go4. Beides muss einzeln kompiliert werden:

Klone Projekt:

	git clone http://gitlab.ikp.physik.tu-darmstadt.de/omoeller/trajektorienrekonstruktion.git

Zuerst die Analyse kompilieren:

	cd trajektorienrekonstruktion/build
	rm -rf *
	cmake ..
	make -j

Dann die Uebergabe an go4:

	cd trajektorienrekonstruktion/go4/build
	rm -rf *
	cmake ..
	make

Nun kann das Programm - etwa ueber Go4 - gestartet werden. Die in go4 zu ladende library liegt hier:

	trajektorienrekonstruktion/go4/build/libGo4UserAnalysis.so



## Usage
- trackingprogramm Driftkammer 
- evtl TODO: Bsps

## Support
- ...

## Roadmap
- TrackFitting implementieren -> verschiedene moeglichkeiten (Genfit KlamanFitter, ...; 'alter' Algorithmus)
- auch TrackFitting konfigurierbar machen
- RootBrowser-Analyse wieder hinkriegen -> konfigurierbar machen, ob RooTBrowser/Go4
- online DataReader schreiben --> nutze elderPT/Go4MBSReader??
- stream-Reader schreiben --> nutze stream?
- koinzidenten branch zu e- tracking schreiben (gammas) -> speichere in jedem branch eventID -> fuehre darueber spaeter zusammen
- fuer koinz. branch: konfigurierbar machen, welche(n) benutzen; Reader + RawEvent muessen erweitert werden
- output klassen schreiben -> fuer spektrum etc
- in config histogramme definieren koennen -> brauche einlese-klasse
- alles ordnen, dokumentieren, kommentieren...
- mit joern & michael sprechen --> habe ich mist verzapft?

## Contributing
State if you are open to contributions and what your requirements are for accepting them.

For people who want to make changes to your project, it's helpful to have some documentation on how to get started. Perhaps there is a script that they should run or some environment variables that they need to set. Make these steps explicit. These instructions could also be useful to your future self.

You can also document commands to lint the code or run tests. These steps help to ensure high code quality and reduce the likelihood that the changes inadvertently break something. Having instructions for running tests is especially helpful if it requires external setup, such as starting a Selenium server for testing in a browser.

## Authors and acknowledgment
Show your appreciation to those who have contributed to the project.

## License
For open source projects, say how it is licensed.

## Project status
If you have run out of energy or time for your project, put a note at the top of the README saying that development has slowed down or stopped completely. Someone may choose to fork your project or volunteer to step in as a maintainer or owner, allowing your project to keep going. You can also make an explicit request for maintainers.
