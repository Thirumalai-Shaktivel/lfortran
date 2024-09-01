program expr2
implicit none

integer, allocatable :: x(:)
allocate(x(4))
! x = 123
! print *, x
x(1) = 123
x(2) = 456
x(3) = 789
x(4) = 101
print *, x(1)
print *, x(2)
print *, x(3)
print *, x(4)
end program
