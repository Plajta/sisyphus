"use client";

import { exportSheet } from "~/actions/sheet/export";
import { Button } from "~/components/ui/button";

interface ExportSheetButtonProps {
	id: number;
}

export function ExportSheetButton({ id }: ExportSheetButtonProps) {
	return (
		<Button
			onClick={async () => {
				const response = await exportSheet({ id });

				if (response && response.data && response.data.redirect) {
					window.open(`http://localhost:3000${response.data.redirect}`, "_blank");
				}
			}}
		>
			Exportovat konfiguraci
		</Button>
	);
}
