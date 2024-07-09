#include <global.h>

#include "examplegame.h"




extern "C"
int main( int argc, char * argv [] )
try
{
	std::ios_base::sync_with_stdio(false);
	ExampleGame game;
	return game.Run();
}
catch(const std::exception &e )
{
	println( "Non-SDL Exception has occurred!\n" );
	println("Fehler: {}\n", e.what());

	// put a breakpoint in this line, to prevent the shell from closing
	return 66;
}
