"use server";

import { z } from "zod";
import { zfd } from "zod-form-data";
import { actionClient } from "~/modules/safe-action";
import { writeFile } from "node:fs/promises";
import { prisma } from "~/modules/prisma";
import { revalidatePath } from "next/cache";

const path = `${process.cwd()}/files`;

const schema = zfd.formData({
	id: zfd.text(z.coerce.number()),
	text: zfd.text(z.string()).optional(),
	image: zfd.file(z.instanceof(File).optional()),
	voice: zfd.file(z.instanceof(File).optional()),
});

export const editButton = actionClient.schema(schema).action(async ({ parsedInput: { id, text, image, voice } }) => {
	if (text) {
		await prisma.button.update({
			where: {
				id,
			},
			data: {
				text,
			},
		});

		revalidatePath("/dashboard/configuration");
	}

	if (image) {
		await prisma.file.deleteMany({
			where: {
				type: "IMAGE",
				buttonId: id,
			},
		});

		const fileRecord = await prisma.file.create({
			data: {
				type: "IMAGE",
				buttonId: id,
			},
		});

		try {
			await writeFile(`${path}/${fileRecord.id}.png`, Buffer.from(await image.arrayBuffer()));

			revalidatePath("/dashboard/configuration");
		} catch (e) {
			return { success: true, message: e };
		}
	}

	if (voice) {
		await prisma.file.deleteMany({
			where: {
				type: "VOICE",
				buttonId: id,
			},
		});

		const fileRecord = await prisma.file.create({
			data: {
				type: "VOICE",
				buttonId: id,
			},
		});

		try {
			await writeFile(`${path}/${fileRecord.id}.wav`, Buffer.from(await voice.arrayBuffer()));

			revalidatePath("/dashboard/configuration");
		} catch (e) {
			return { success: true, message: e };
		}
	}

	return { success: true, message: "Tlačítko úspěšně upraveno" };
});
