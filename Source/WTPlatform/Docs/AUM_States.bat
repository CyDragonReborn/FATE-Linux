@ECHO Generates the AUM.ps and AUM.gif files -- pictures of the
@ECHO state diagrams for the Application Update Manager.
@ECHO .
@ECHO Uses the 'dot' language to describe the state transition graph.

set F=AUM_States
dot %F%.dot -Tps -o %F%.ps
dot %F%.dot -Tgif -o %F%.gif

start %F%.ps
REM start %F%.gif
pause

