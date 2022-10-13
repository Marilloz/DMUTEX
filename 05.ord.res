# Ejemplo con multiples semaforos
# Procesos: A,B,C,D
# Semaforos: X,Y
A: [EVENT]-> A{TICK} B{--} C{--} D{--}
B: [EVENT]-> A{--} B{TICK} C{--} D{--}
B: [EVENT]-> A{--} B{TICK} C{--} D{--}
C: [EVENT]-> A{--} B{--} C{TICK} D{--}
D: [EVENT]-> A{--} B{--} C{--} D{TICK}
A: [GETCLOCK]-> A{LC[1,0,0,0]} B{--} C{--} D{--}
B: [GETCLOCK]-> A{--} B{LC[0,2,0,0]} C{--} D{--}
C: [GETCLOCK]-> A{--} B{--} C{LC[0,0,1,0]} D{--}
D: [GETCLOCK]-> A{--} B{--} C{--} D{LC[0,0,0,1]}
# Los procesos A y B intentan cerrar X e Y, respectivamente
A: [LOCK X]-> A{TICK|SEND(LOCK,B)|SEND(LOCK,C)|SEND(LOCK,D)} B{--} C{--} D{--}
B: [LOCK Y]-> A{--} B{TICK|SEND(LOCK,A)|SEND(LOCK,C)|SEND(LOCK,D)} C{--} D{--}
# Todos reciben las peticiones
B: [RECEIVE]-> A{--} B{RECEIVE(LOCK,A)|TICK|TICK|SEND(OK,A)} C{--} D{--}
C: [RECEIVE]-> A{--} B{--} C{RECEIVE(LOCK,A)|TICK|TICK|SEND(OK,A)} D{--}
D: [RECEIVE]-> A{--} B{--} C{--} D{RECEIVE(LOCK,A)|TICK|TICK|SEND(OK,A)}
A: [RECEIVE]-> A{RECEIVE(LOCK,B)|TICK|TICK|SEND(OK,B)} B{--} C{--} D{--}
C: [RECEIVE]-> A{--} B{--} C{RECEIVE(LOCK,B)|TICK|TICK|SEND(OK,B)} D{--}
D: [RECEIVE]-> A{--} B{--} C{--} D{RECEIVE(LOCK,B)|TICK|TICK|SEND(OK,B)}
# Tanto A como B deben recibir los 3 OKs
A: [RECEIVE]-> A{RECEIVE(OK,B)|TICK} B{--} C{--} D{--}
A: [RECEIVE]-> A{RECEIVE(OK,C)|TICK} B{--} C{--} D{--}
A: [RECEIVE]-> A{RECEIVE(OK,D)|TICK|MUTEX(X)} B{--} C{--} D{--}
# A cierra X
B: [RECEIVE]-> A{--} B{RECEIVE(OK,A)|TICK} C{--} D{--}
B: [RECEIVE]-> A{--} B{RECEIVE(OK,C)|TICK} C{--} D{--}
B: [RECEIVE]-> A{--} B{RECEIVE(OK,D)|TICK|MUTEX(Y)} C{--} D{--}
# B cierra Y
A: [EVENT]-> A{TICK} B{--} C{--} D{--}
B: [EVENT]-> A{--} B{TICK} C{--} D{--}
# Ahora liberamos X E Y
A: [UNLOCK X]-> A{--} B{--} C{--} D{--}
B: [UNLOCK Y]-> A{--} B{--} C{--} D{--}
# ##############################
# REPETIMOS LO MISMO CON C Y D #
# ##############################
C: [LOCK Y]-> A{--} B{--} C{TICK|SEND(LOCK,A)|SEND(LOCK,B)|SEND(LOCK,D)} D{--}
D: [LOCK X]-> A{--} B{--} C{--} D{TICK|SEND(LOCK,A)|SEND(LOCK,B)|SEND(LOCK,C)}
# Todos reciben las peticiones
C: [RECEIVE]-> A{--} B{--} C{RECEIVE(LOCK,D)|TICK|TICK|SEND(OK,D)} D{--}
B: [RECEIVE]-> A{--} B{RECEIVE(LOCK,C)|TICK|TICK|SEND(OK,C)} C{--} D{--}
A: [RECEIVE]-> A{RECEIVE(LOCK,C)|TICK|TICK|SEND(OK,C)} B{--} C{--} D{--}
D: [RECEIVE]-> A{--} B{--} C{--} D{RECEIVE(LOCK,C)|TICK|TICK|SEND(OK,C)}
B: [RECEIVE]-> A{--} B{RECEIVE(LOCK,D)|TICK|TICK|SEND(OK,D)} C{--} D{--}
A: [RECEIVE]-> A{RECEIVE(LOCK,D)|TICK|TICK|SEND(OK,D)} B{--} C{--} D{--}
# Tanto C como D deben recibir los 3 OKs
D: [RECEIVE]-> A{--} B{--} C{--} D{RECEIVE(OK,C)|TICK}
D: [RECEIVE]-> A{--} B{--} C{--} D{RECEIVE(OK,B)|TICK}
D: [RECEIVE]-> A{--} B{--} C{--} D{RECEIVE(OK,A)|TICK|MUTEX(X)}
# D cierra X
C: [RECEIVE]-> A{--} B{--} C{RECEIVE(OK,B)|TICK} D{--}
C: [RECEIVE]-> A{--} B{--} C{RECEIVE(OK,A)|TICK} D{--}
C: [RECEIVE]-> A{--} B{--} C{RECEIVE(OK,D)|TICK|MUTEX(Y)} D{--}
# C cierra Y
D: [EVENT]-> A{--} B{--} C{--} D{TICK}
C: [EVENT]-> A{--} B{--} C{TICK} D{--}
# Ahora liberamos X E Y
D: [UNLOCK X]-> A{--} B{--} C{--} D{--}
C: [UNLOCK Y]-> A{--} B{--} C{--} D{--}
# Terminando ¿Valores de LCs?
A: [GETCLOCK]-> A{LC[12,5,6,6]} B{--} C{--} D{--}
B: [GETCLOCK]-> A{--} B{LC[4,13,6,6]} C{--} D{--}
C: [GETCLOCK]-> A{--} B{--} C{LC[10,11,12,8]} D{--}
D: [GETCLOCK]-> A{--} B{--} C{--} D{LC[12,13,8,12]}
