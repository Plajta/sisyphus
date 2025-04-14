import { Prisma } from "@prisma/client";
import { Edit, Volume2 } from "lucide-react";
import Image from "next/image";
import Link from "next/link";
import { Button } from "~/components/ui/button";
import { Card, CardHeader, CardContent } from "~/components/ui/card";
import { Separator } from "~/components/ui/separator";
import { VoiceButton } from "~/components/voice-button";

export function ImageCard({
	item: { id, text, files },
	sheetId,
}: {
	item: Prisma.ButtonGetPayload<{ include: { files: true } }>;
	sheetId: number;
}) {
	const image = files.find((file) => file.type === "IMAGE");

	const voice = files.find((file) => file.type === "VOICE");

	return (
		<Card style={{ height: 280 }}>
			<CardHeader className="flex justify-center">
				<div style={{ height: 150 }}>
					{image ? (
						<Image src={`/api/file/${image.id}`} width={150} height={100} alt={`tlacitko-${image.id}`} />
					) : (
						<p>Není nahrán obrázek</p>
					)}
				</div>
			</CardHeader>

			<Separator />

			<CardContent>
				<div className="flex flex-col gap-4">
					<div className="flex justify-between items-center">
						<p className="text-lg">{text}</p>

						<div className="flex gap-1">
							<VoiceButton source={voice && `/api/file/${voice.id}`} />

							<Link href={`/dashboard/configuration/sheet/${sheetId}/dialog/button/${id}`}>
								<Button variant="outline">
									<Edit />
								</Button>
							</Link>
						</div>
					</div>
				</div>
			</CardContent>
		</Card>
	);
}
