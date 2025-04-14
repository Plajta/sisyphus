"use client";

export default function Page() {
	return (
		<div className="flex flex-1 flex-col gap-4 px-4 py-10 justify-center items-center">
			<div className="flex flex-col">
				<p className="text-xl font-bold text-center">Komunikátor není připojen.</p>
				<p className="text-muted-foreground text-sm">
					Pro konfiguraci komuníkátoru prosím ho prosím připojte do USB díry, díks
				</p>
			</div>
		</div>
	);
}
