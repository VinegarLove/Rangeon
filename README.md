# Rangeon
## Simple maze-builder with rooms and doors.

It use a brute forcing tecnique using room carving tecniques and clustering.

Altering the code one can specify the following parameters. I did not implement the option to do this as standard input.

- Max size of the rooms
- Retry room count (how many time to brute force a room inside the maze)
- Percentage of how much the maze should be "perfect" vs avoiding dead ending
- Percentage of how much the maze should be wind around (% of having corner on corridor).

It outputs a text file with the layout, as well as displaying the output

The symbol # displays the walls, the letter "O" displays the doors and the "\_" are rooms and corridors

An example can be found in the repo.
