#include <global.h>

#include "examplegame.h"




extern "C"
int main( int argc, char * argv [] )
try
{
	std::ios_base::sync_with_stdio(false);
	BzlGame::ExampleGame game;
	return game.Run();
}
catch(const std::exception &e )
{

	BzlGame::println( "Non-SDL Exception has occurred!\n" );
	BzlGame::println("Fehler: {}\n", e.what());

	// put a breakpoint in this line, to prevent the shell from closing
	return 66;
}
