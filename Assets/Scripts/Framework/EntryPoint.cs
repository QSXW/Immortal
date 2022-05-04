using Immortal;

class EntryPoint
{
    static void Main(string[] args)
    {
        Log.Info("Init with succeed");
        
        for (int i = 0; i < args.Length; i++)
		{
            Log.Info("argv[" + i + "]: " + args[i]);
        }
	}
}
