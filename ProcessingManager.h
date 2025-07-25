#pragma once
#include "Module.h"

/*
	ProcessingManager is responsible for ordering modules for processing, in accordance to their dependencies.
*/
class ProcessingManager {

	/*
		For context, a set of modules may have the following topology...

		+-------------+        +--------------+
		| Oscillator  |        |  Envelope    | 
		+-------------+        +--------------+
				|                     |
				+----------+----------+
						   v
					+-------------+
					|     Amp     |
					+-------------+
						   v
					+-------------+
					|   Reverb    |
					+-------------+



		The *sortedModules* array may then be structured as [Oscillator, Envelope, Amp, Reverb]

		Modules write to their output buffer, which is then copied to the subsequent input buffers in the chain

		so in this case... 
			-	Oscillator and Envelope populates Amp's input buffer
			-	Amp populates Reverb's input buffer
	*/


	public:

		/* registers a module (all modules are registered when created */
		void AddModule(Module* module);

		/* removes a module (called when a module is deleted) */
		void RemoveModule(Module* module);

		/*
			registers a connection 

			@param outModule		-	the module which contains an out socket
			@param outSocketIndex	-	the index of the output socket
			@param inModule			-	the module containing the in socket
			@param inSocketIndex	-	the index of the input socket
		*/
		void AddConnection(ConnectionType connectionType, Module* outModule, int outSocketIndex, Module* inModule, int inSocketIndex);

		/* removes a registered connection */
		void RemoveConnection(ConnectionType connectionType, Module* outModule, int outSocketIndex, Module* inModule, int inSocketIndex);

		/* sorts modules in the order they should be processed (computing by  each modules connections) */
		void SortModules();

		/* prints the the order of the sorteed modules */
		void PrintOrdering();
		void PrintConnections();

		/* calls prepare on all blocks, should be called before each DSP cycle */
		void PrepareAll(double sampleRate, int blockSize);
		/* calls process on each module, ensuring the appropriate ordering */
		void ProcessAll();




	private:

		/* processing an indivudal module */
		void ProcessModule(Module* module);

		struct Connection {
			
			ConnectionType connectionType;

			Module* outModule;
			int outSocketIndex;
			
			Module* inModule;
			int inSocketIndex;



			/* Two connections are equal if they have the same parameters */
			bool operator==(const Connection& other) const {
				return outModule == other.outModule && 
					outSocketIndex == other.outSocketIndex &&
					inModule == other.inModule &&
					inSocketIndex == other.inSocketIndex;
			}
		};

		std::vector<Module*> allModules;
		std::vector<Module*> sortedModules;
		
		std::vector<Connection> connections;
};