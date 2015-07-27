#!/bin/csh
	echo "`date` : Starting up a new loop"
	rm DpmRunnumber.list  
	ls -d /global/homes/m/mlomnitz/mlomnitz_projectdir/michael/production/2015-07-23_09-33.picoHFtree/*/* --color=none | cut -d / -f 11 >DpmRunnumber.list
	set nRunnumbers = `cat DpmRunnumber.list`
	echo "nRuns" $#nRunnumbers
	set i = 1
	while ( $i <= $#nRunnumbers )
	set iRunnumber = `echo $nRunnumbers[$i]`
	@ i = $i + 1
	set iDay = `echo $iRunnumber | awk '{print substr($0,3,3)}'`
	#set iDay = `echo $iRunnumber`
	echo $iDay $iRunnumber
	#this flag1 is to confirm an old filelis, if iFlag1==0,then need run, if iFlag==1 &&iFlag3==1,need rerun
	if ( -f ./RunnumberDpmFileList/${iRunnumber}.list ) then
	rm ./RunnumberDpmFileList/${iRunnumber}.list
	ls -d /global/homes/m/mlomnitz/mlomnitz_projectdir/michael/production/2015-07-23_09-33.picoHFtree/${iDay}/${iRunnumber}/*picoHFtree.root > ./RunnumberDpmFileList/${iRunnumber}.list
	else
	ls -d /global/homes/m/mlomnitz/mlomnitz_projectdir/michael/production/2015-07-23_09-33.picoHFtree/${iDay}/${iRunnumber}/*picoHFtree.root > ./RunnumberDpmFileList/${iRunnumber}.list
	endif
	
	end
  rm DpmProd.All.D0.list
  cat ./RunnumberDpmFileList/*.list >DpmProd.All.D0.list
