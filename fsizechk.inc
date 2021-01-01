fsize .equ $-ProgStart
  .echo " -- Output Size: "
  .echo fsize
  .echo " B -- \n"
#if fsize > 8811
  .error ; not a real directive -- throws an error !
  .echo " !!! ERROR: Filesize has exceeded maximum !\n"
#endif
