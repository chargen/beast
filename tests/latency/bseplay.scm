;; CC0 Public Domain: http://creativecommons.org/publicdomain/zero/1.0/
;; bseplay.scm - silently play .bse files		-*- scheme -*-

;; play argc/argv contents as bse files
(define (bse-play)
  (bse-server-register-blocking bse-server-register-core-plugins #f)
  ; (bse-server-register-blocking bse-server-register-scripts #f)
  ; (bse-server-register-blocking bse-server-register-ladspa-plugins #f)
  (map (lambda (file)
	 (let ((project (bse-server-use-new-project bse-server file))
	       (blimp "*"))
	   (bse-project-restore-from-file project file)
	   (bse-project-play project)
	   ;; (display file)
	   ;; (display ": -")
	   (while (bse-project-is-playing project)
		  (usleep 250000)
		  ;; (display #\backspace)
		  ;; (display blimp)
		  (if (string=? blimp "*")
		      (set! blimp "o")
		      (set! blimp "*")))
	   (bse-project-stop project)
	   (bse-item-unuse project)
	   ;; (display #\backspace)
	   ;; (display "done.")
	   ;; (newline)
	   ))
       (cdr (command-line))))
(bse-play)
