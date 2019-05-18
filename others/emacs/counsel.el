;;** `counsel-fast-grep'
(defcustom counsel-fast-grep-base-command "fast-grep -n -p '[.](c|cc|cpp|h|hpp|hh|md|org|txt)$' %s ."
  "Alternative to `counsel-ag-base-command' using fast-grep"
  :type 'string)

(counsel-set-async-exit-code 'counsel-fast-grep 1 "No matches found")
(ivy-set-occur 'counsel-fast-grep 'counsel-ag-occur)
(ivy-set-display-transformer 'counsel-fast-grep 'counsel-git-grep-transformer)

;;;###autoload
(defun counsel-fast-grep (&optional initial-input initial-directory extra-rg-args rg-prompt)
  "Grep for a string in the current directory using fast-grep."
  (interactive)
  (let ((counsel-ag-base-command counsel-fast-grep-base-command)
        (counsel--grep-tool-look-around
         (let ((rg (car (split-string counsel-fast-grep-base-command)))
               (switch "--pcre2"))
           (and (eq 0 (call-process rg nil nil nil switch "--version"))
                switch))))
    (counsel-ag initial-input initial-directory extra-rg-args rg-prompt
                :caller 'counsel-fast-grep)))
(cl-pushnew 'counsel-fast-grep ivy-highlight-grep-commands)

;;** `counsel-codesearch'
(defcustom counsel-codesearch-base-command "codesearch -n -p '[.](c|cc|cpp|h|hpp|hh|md|org|txt)$' %s"
  "Search for a text pattern using codeseach command."
  :type 'string)

(counsel-set-async-exit-code 'counsel-codesearch 1 "No matches found")
(ivy-set-occur 'counsel-codesearch 'counsel-ag-occur)
(ivy-set-display-transformer 'counsel-codesearch 'counsel-git-grep-transformer)

;;;###autoload
(defun counsel-codesearch (&optional initial-input initial-directory extra-rg-args rg-prompt)
  "Grep for a string in the current directory using codesearch."
  (interactive)
  (let ((counsel-ag-base-command counsel-codesearch-base-command)
        (counsel--grep-tool-look-around
         (let ((rg (car (split-string counsel-codesearch-base-command)))
               (switch "--pcre2"))
           (and (eq 0 (call-process rg nil nil nil switch "--version"))
                switch))))
    (counsel-ag initial-input initial-directory extra-rg-args rg-prompt
                :caller 'counsel-codesearch)))
(cl-pushnew 'counsel-codesearch ivy-highlight-grep-commands)

;; For fast-locate command
(defun counsel-locate-cmd-default (input)
  "Return a shell command based on INPUT."
  (counsel-require-program "fast-locate")
  (format "fast-locate '%s'"
          (counsel--elisp-to-pcre
           (ivy--regex input))))

(defun counsel-locate-cmd-noregex (input)
  "Return a shell command based on INPUT."
  (counsel-require-program "fast-locate")
  (format "fast-locate '%s'" input))
