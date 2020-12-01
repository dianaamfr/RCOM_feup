# RCOM_feup

# Porta de Série Virtual

sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777
Emissor ligado a ttyS10
Recetor ligado a ttyS11
