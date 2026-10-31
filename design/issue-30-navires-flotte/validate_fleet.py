#!/usr/bin/env python3
"""G02 #30: verification de la geometrie de la flotte de demonstration."""
P=[("porte-avions",5,1,1,"H"),("sous-marin",3,2,5,"H"),
   ("contre-torpilleur",3,5,4,"V"),("croiseur",4,8,3,"V"),
   ("patrouilleur",2,6,7,"H")]
cells={}
for name,n,c,r,d in P:
    for i in range(n):
        xy=(c+(i if d=="H" else 0),r+(i if d=="V" else 0))
        assert 0<=xy[0]<10 and 0<=xy[1]<10,(name,xy)
        assert xy not in cells,(cells[xy],name,xy)
        cells[xy]=name
assert len(P)==5 and sorted(n for _,n,_,_,_ in P)==[2,3,3,4,5]
assert len(cells)==17
assert {o for *_,o in P}=={"H","V"}
assert cells[(4,5)]=="sous-marin" and cells[(5,5)]=="contre-torpilleur"
assert cells[(7,7)]=="patrouilleur" and cells[(8,6)]=="croiseur"
print("PASS G02: 5 navires, 17 cases, H/V, contact cote et diagonale.")
