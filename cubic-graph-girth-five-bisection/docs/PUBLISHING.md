# Publication checklist

1. Use the paper-level repository `whausmann/bisections-code`. Upload this entire directory as the subfolder `cubic-graph-girth-five-bisection`, alongside the other code package. Keep its internal file structure intact.
2. The paper-level README should link to both packages and explain which results each verifies. Add repository-wide citation metadata at the root when both packages are assembled; this subfolder's CITATION.cff describes this package alone.
3. Links assume the default branch is `main`. Update them if another branch name is used. Confirm the package link after upload: https://github.com/whausmann/bisections-code/tree/main/cubic-graph-girth-five-bisection .
4. Add chip, macOS, compiler and shortg versions to docs/PROVENANCE.md if recoverable. Do not invent unavailable version information.
5. Create a tagged release of the paper-level repository when both packages are ready. If archiving on Zenodo, use the version-specific DOI in the paper citation. No release or DOI has been created by this packaging task.
6. Add docs/citation.bib to the paper bibliography and paste docs/paper-snippet.tex into the paper. The current bibliography entry cites this package; it can be replaced by a repository-wide software citation once both packages are assembled.

Large caches, binaries, and subsequent generated runs are excluded by this package's .gitignore. The included small historical results are intended to be tracked. Regenerate SHA256SUMS after edits, excluding SHA256SUMS itself. Core source and runner files are unchanged from the supplied run folder.
