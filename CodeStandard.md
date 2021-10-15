Kodstandard:

- Use namespace.
Men helst inte, typ, globalt? (Rikard får förklara)

- Remember to comment what your functions do, it isn't obvious for everyone.

- Simple math. = sm
- Dirextx = dx

- Måsvingar på separata rader.
	if ()
	{
		do things
	}

- Stor bokstav på alla funktioner.
	void DoThings();

- m_variabelnamn.

- Struct _t
	struct camera_Matrix_t
	{
		sm::Matrix projection;
		sm::Matrix view;
	};

CPU/GPU:
Behövs ingen transponering på CPU:n 
På GPU är det därför viktigt att vi kör nyVektor = mul(inMatris, inVektor); 

-Bool
	- isDoingSomething
	- hasDoneSomething

- Finns en .cpp fil, gör alla funktioner där oavsett storlek.

- Private först i klasser.
	privater:
		Variabler
	public:
		Funktioner

- Använd smartpekare (Com_ptr<>, unique_ptr<>, shared-ptr<>), om du inte kan, se till att du tar bort minnesluckor MED EN GÅNG.

- Stäng av varningar för externa bibliotek (om du vill c:)

- Använd logiska variabler i for-loopar.

- Gör skit user friendly.