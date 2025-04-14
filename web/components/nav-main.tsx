"use client";

import { Collapsible } from "@radix-ui/react-collapsible";
import { ChevronRight, type LucideIcon } from "lucide-react";
import Link from "next/link";
import { useParams, usePathname } from "next/navigation";
import { Serial } from "~/components/serial";
import { Button } from "~/components/ui/button";

import { CollapsibleContent, CollapsibleTrigger } from "~/components/ui/collapsible";

import {
	SidebarGroup,
	SidebarMenu,
	SidebarMenuAction,
	SidebarMenuButton,
	SidebarMenuItem,
	SidebarMenuSub,
	SidebarMenuSubButton,
	SidebarMenuSubItem,
} from "~/components/ui/sidebar";

export interface NavItem {
	title: string;
	url: string;
	description: string;
	icon: LucideIcon;
	subItems: { title: string; url: string }[];
}

export function NavMain({ items }: { items: NavItem[] }) {
	const pathname = usePathname();
	const params = useParams();

	return (
		<SidebarGroup>
			<div className="flex flex-col gap-1">
				<Serial />

				<Link className="mb-2" href={`/dashboard/configuration/sheet/${params.sheetId}/dialog/sheet/new`}>
					<Button className="w-full" variant="outline">
						Přidat Konfiguraci
					</Button>
				</Link>
			</div>

			<SidebarMenu>
				{items.map((item) => (
					<Collapsible
						key={item.title}
						asChild
						defaultOpen={item.url === pathname.replace(`/${params.sheetId}`, "")}
					>
						<SidebarMenuItem>
							<SidebarMenuButton asChild tooltip={item.title}>
								<a href={item.url}>
									<item.icon />
									<span>{item.title}</span>
								</a>
							</SidebarMenuButton>
							{item.subItems?.length ? (
								<>
									<CollapsibleTrigger asChild>
										<SidebarMenuAction className="data-[state=open]:rotate-90">
											<ChevronRight />
											<span className="sr-only">Toggle</span>
										</SidebarMenuAction>
									</CollapsibleTrigger>
									<CollapsibleContent>
										<SidebarMenuSub>
											{item.subItems?.map((subItem) => (
												<SidebarMenuSubItem key={subItem.title}>
													<SidebarMenuSubButton asChild isActive={pathname === subItem.url}>
														<a href={subItem.url}>
															<span>{subItem.title}</span>
														</a>
													</SidebarMenuSubButton>
												</SidebarMenuSubItem>
											))}
										</SidebarMenuSub>
									</CollapsibleContent>
								</>
							) : null}
						</SidebarMenuItem>
					</Collapsible>
				))}
			</SidebarMenu>
		</SidebarGroup>
	);
}
