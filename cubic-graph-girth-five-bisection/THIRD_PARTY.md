# Third-party tools

The MIT license covers only this project's original code and documentation. No third-party source or binary is included in this distribution.

- **Snarkhunter**, Gunnar Brinkmann, Jan Goedgebeur and Brendan D. McKay: https://caagt.ugent.be/cubic/ . The upstream project identifies its license as GNU GPL. The setup script downloads version 2.0b; its downloaded source and license remain governed by upstream terms. See also G. Brinkmann, J. Goedgebeur and B. D. McKay, *Generation of cubic graphs*, Discrete Mathematics & Theoretical Computer Science 13(2), 69–80 (2011).
- **nauty / shortg**, upstream project: https://users.cecs.anu.edu.au/~bdm/nauty/ . The setup downloads nauty 2.9.3 to build the generator, while the runner uses an independently installed shortg. Retain and comply with the license files supplied with each dependency if redistributing them. This repository does not relicense either dependency.

Official generator usage was checked against https://caagt.ugent.be/cubic/README.txt during packaging. Dependencies are obtained externally, not vendored or cryptographically pinned by the historical setup script.
