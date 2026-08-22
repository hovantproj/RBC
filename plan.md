# Plan

How do we assemble motors??
- stalked other teams and figured it out from the picture (we were just not organising our parts)

Organised parts

## Motors
Put motors on; need to test the following:
- [X] L298N motor module works as expected (along with code)
- [X] base setup works
- [ ] holy it does not drive straight, too much tolerance, need to find a way to keep it straight... hot glue?
    - [ ] try calibrate left and right speeds to be slightly diff so it drives straight
- [ ] code 90deg turn left and right

## Grippers
Plan out the gripper design
- the animals are really small and are light as
- interlocking arms, just reach for everything in front and pull it in and secure it?
- [ ] will need to test that they properly secure animals

## Path (animal) Finding
the coordinates are the position of the rfid tag in the grid
which are 10cm apart
the start is (0,0)
and the grid is 21x13

## Overall Design
Figure out how everything will sit together...
- [ ] figure out how to mount the gripper onto the robot
- [ ] decide where the arduino will sit (powered by L298N module?)
- [ ] also need a space for the 