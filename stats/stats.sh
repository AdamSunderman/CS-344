#!/bin/bash

#—STATS—Finds Averages and Medians on Columns or Rows of Text Files or Streams
#Written By Adam Sunderman (sunderad@oregonstate.edu) for CS-344 Oregon State University
#               *LastEdit* :10/11/16 
# !columns of data are expected to be separated by tab and rows are to be separated by return
#

inputroute="datafile$$"            #Route to input file
isfile="none"                      #Holds a value to see if the program reads from file or stdin
linesize=0                         #Length of input data rows
colsize=0                          #Length of input data columns (creates a sudo-matrix for data paired with linesize)
declare -a lines                   #If {-rows} option is used lines of input data will be stored as rows
declare -a cols                    #If {-cols} option is used columns of input data will be stored as rows  
option="none"                      #Holds the option used either "none", "rows" or "cols" 
returnvar=0                        #Holds return values for average and median functions

trap "rm -f $inputroute; exit 1" INT HUP TERM KILL

#Calculates averages by line and sends the average to the global variable returnvar
function average {
   sum=0
   avg=0
   for num in $@
   do
      sum=`expr $sum \+ $num`
   done
   returnvar=`expr \( $sum \+ \( $# \/ 2 \) \) \/ $#`
}

#Calculates madian by line and sends the median to the global variable returnvar
function median {
   orderedlist=$(echo "$@"|tr " " "\n"|sort -n|tr "\n" " ")        #sort the line

   if [ `expr $# \/ 2` == "0" ]                              #check if there is an even number of values
   then
      index=`expr $# \/ 2`                                   #if there is an even number of values
      altindex=`expr $index \+ 1`                            #get the index of the two middle values,
      counter=0                                              #add them together and store them in returnvar
      for mnum in $orderedlist
      do
         if [ $counter == $index ]
         then
            returnvar=$mnum
            let "counter=counter+1"
         elif [ $counter == $altindex]
         then
            if [ $returnvar < $mnum ]
            then
               returnvar=$mnum
               break
            fi
         else
            let "counter=counter+1"
         fi     
      done
   else                                                      #if there is an odd number of values
      index=`expr $# \/ 2`                                   #just get the middle value and store it in returnvar
      counter=0
      for mnum in $orderedlist
      do
         if [ $counter == $index ]
         then
            returnvar=$mnum
            break
         else
            let "counter=counter+1"
         fi  
         
      done
   fi
}


#Check that the script has been called with at least one, and no extra, parameters.
if [ "$#" -eq "0" ] || [ "$#" -gt "2" ]
then
   echo "Usage: stats {-rows|-cols} [file]"
   exit 1
fi


#Check for the -[c]olumns or -[r]ows option in script usage
if [[ $1 == -r* ]]
then 
   option="rows"
elif [[ $1 == -c* ]]
then
   option="cols"
else
   echo "Usage: stats {-rows|-cols} [file]"
   exit 1
fi


#Check if there is a file specified for input and set the dataroute to a tempfile
if [ "$#" -eq "1" ]
then
   isfile="false"
   cat > "$inputroute"
elif [ "$#" -eq "2" ]
then
   isfile="true"
   inputroute=$2
   test -e "$inputroute"        #see if file exists
   if [ $? -ne "0" ]
   then
      echo "Bad input"
      exit 1
   fi
   test -s "$inputroute"        #see if file exists
   if [ $? -ne "0" ]
   then
      echo "Bad input"
      exit 1
   fi
fi

#Test the input file

#Read all line data into an array (lines) and count data size (row length, column length)
while read myLine
do
   lines[$colsize]=$myLine
   linesize=0
   colsize=`expr $colsize \+ 1`       	   #count input column length
   
   for i in $myLine
   do
      linesize=`expr $linesize \+ 1`       #count input row length
   done
   
done <$inputroute


#Print the averages and medians of all rows or columns. All calculations are done on rows
#of data. If the {-cols} option is used, first convert the columns into rows of data. If  
#the {-rows} option is used no conversion is needed.
if [ "$option" != 'rows' ]
then
   for i in "${lines[@]}"
   do
      colindex=0
      for j in $i
      do
         temp=${cols[$colindex]}           #get previous stored column data
         temp="$temp $j"                   #append new data
         cols[$colindex]=$temp             #replace previous stored column data 
         let "colindex=colindex+1"
      done
   done
   
   echo Averages:                          #calculate averages by line and print to screen
   for aval in "${cols[@]}"
   do
      average $aval
      echo -ne $returnvar '\t' 
   done
   echo -ne '\n'

   echo Medians:                           #calculate medians by line and print to screen
   for mval in "${cols[@]}"
   do
      median $mval
      echo -ne $returnvar '\t' 
   done
   echo -ne '\n'

else
   echo Averages  Medians                  #calculate averages and medians by line and print to screen
   for val in "${lines[@]}"
   do
      average $val
      echo -ne $returnvar '\t'
      median $val 
      echo -ne $returnvar '\t'
      echo -ne '\n'
   done
  
fi

#If there is a temp file remove it  
if [ "$isfile" != 'true' ]
then
   rm -f $inputroute 
fi







