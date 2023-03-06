# Kosmos-ethynyl-radical
Goals:
kosmos-sem.c solves the Kosmos-ethynyl-radical
problem with the use of semaphores.

kosmos-mcv.c solves the Kosmos-ethynyl-radical
problem with the use of mutexes and condition variables.

The Kosmos-ethynyl radical problem
You are hired by an interstellar civilzation of much greater intelligence that
ours (!!) who are seeding the universe with the building blocks needed for a
one of their major projects. The specific task you have agreed to help solve
for them is to manage the chemical reaction needed to form ethynyl radicals
(which is made up of two carbon atoms and one hydrogen atom).
Despite the civilization's greater intelligence, they are having trouble getting
the carbon and hydrogen atoms to combine correctly due to some serious synchronization
problems. They are able to create each atom (i.e., one atom equals one thread), and so
the challenge is to get two carbon threads and one hydrogen thread together at the
same time, regardless of the number or order of thread creation.
Each carbon atom invokes a function named c_ready() when it is ready to react, and
each hydrogen atom invokes a function named h_ready() when it is ready to react.

When an arriving atom/thread determines that a radical can be made, then that
atom/thread must cause the radical to be made to indicate the identities of the
carbon atoms and the identiy of the hydrogen atom are reported. Below is an
example showing the format of the report, where each line indicates the atoms
in a radical, and the atom/thread in parentheses was the one that initiated the
creation of the radical. For example, below the 7th radical consists of carbon
atoms 10 and 14 and hydrogen atom 7, and it was the hydrogen atom that triggered
the creation of the radical.

001: c002 c001 h001 (c002)
002: c004 c009 h002 (h002)
003: c005 c006 h003 (h003)
004: c007 c008 h004 (h004)
005: c003 c013 h005 (h005)
006: c011 c012 h006 (h006)
007: c010 c014 h007 (h007)
008: c016 c015 h008 (c016)
009: c018 c017 h009 (c018)
010: c020 c019 h010 (c020)


Part 1: kosmos-sem.c

Using POSIX semaphores

Part 2: kosmos-mcv.c

Using POSIX mutexes 
