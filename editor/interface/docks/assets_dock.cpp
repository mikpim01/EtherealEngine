#include "assets_dock.h"
#include "runtime/rendering/mesh.h"
#include "runtime/rendering/material.h"
#include "runtime/rendering/texture.h"
#include "runtime/rendering/shader.h"
#include "runtime/assets/asset_manager.h"
#include "runtime/assets/asset_extensions.h"
#include "runtime/ecs/prefab.h"
#include "runtime/ecs/scene.h"
#include "runtime/ecs/utils.h"
#include "runtime/input/input.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/filesystem_watcher.hpp"
#include "../../editing/editing_system.h"
#include "../../system/project_manager.h"
#include "filedialog/filedialog.h"
#include <cstdio>

template<typename T>
asset_handle<texture> get_asset_icon(T asset)
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["folder"];
}

template<>
asset_handle<texture> get_asset_icon(asset_handle<texture> asset)
{
	return asset;
}
template<>
asset_handle<texture> get_asset_icon(asset_handle<prefab> asset)
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["prefab"];
}

template<>
asset_handle<texture> get_asset_icon(asset_handle<scene> asset)
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["scene"];
}

template<>
asset_handle<texture> get_asset_icon(asset_handle<mesh> asset)
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["mesh"];
}
template<>
asset_handle<texture> get_asset_icon(asset_handle<material> asset)
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["material"];
}

asset_handle<texture> get_loading_icon()
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["loading"];
}

template<>
asset_handle<texture> get_asset_icon(asset_handle<shader> asset)
{
	auto& es = core::get_subsystem<editor::editing_system>();
	return es.icons["shader"];
}

asset_handle<texture>& get_icon()
{
	static asset_handle<texture> tex;
	return tex;
}

template<typename T>
void list_entry(
	T& entry,
	const std::string& name,
	bool is_selected,
	bool is_dragging,
	const float size,
	std::function<void()> on_click, 
	std::function<void()> on_double_click,
	std::function<void(const std::string&)> on_rename,
	std::function<void()> on_delete,
	std::function<void()> on_drag)
{
	bool edit_label = false;
	if (is_selected && !gui::IsAnyItemActive())
	{
		
		if (gui::IsKeyPressed(mml::keyboard::F2))
		{
			edit_label = true;
		}

		if (gui::IsKeyPressed(mml::keyboard::Delete))
		{
			if (on_delete)
				on_delete();
		}
	}

	auto get_icon = [](auto entry)
	{
		if (!entry)
			return get_loading_icon();
		
		return get_asset_icon(entry);
	};

	auto icon = get_icon(entry);
	bool loading = !entry;

	gui::PushID(name.c_str());

	if (gui::GetContentRegionAvailWidth() < size)
		gui::NewLine();

	static std::string inputBuff(64, 0);
	std::memset(&inputBuff[0], 0, 64);
	std::memcpy(&inputBuff[0], name.c_str(), name.size() < 64 ? name.size() : 64);

	ImVec2 item_size = { size, size };
	ImVec2 texture_size = item_size;
	if (icon)
		texture_size = { float(icon->info.width), float(icon->info.height) };
	ImVec2 uv0 = { 0.0f, 0.0f };
	ImVec2 uv1 = { 1.0f, 1.0f };

	bool* edit = edit_label ? &edit_label : nullptr;
	int action = gui::ImageButtonWithAspectAndLabel(
		icon.link->asset,
		texture_size, item_size, uv0, uv1,
		is_selected,
		edit,
		loading ? "Loading" : name.c_str(),
		&inputBuff[0],
		inputBuff.size(),
		ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

	if (loading)
	{
		gui::PopID();
		gui::SameLine();
		return;
	}

	if (action == 1)
	{
		if (on_click)
			on_click();
	}
	else if (action  == 2)
	{
		std::string new_name = std::string(inputBuff.c_str());
		if (new_name != name && new_name != "")
		{
			if (on_rename)
				on_rename(new_name);
		}
	}
	else if (action == 3)
	{
		if (on_double_click)
			on_double_click();
	}
	if (gui::IsItemHoveredRect())
	{
		if (gui::IsMouseClicked(gui::drag_button) && !is_dragging)
		{
			if (on_drag)
				on_drag();
		}

	}


	gui::PopID();
	gui::SameLine();

}


template<typename Wrapper, typename T>
int list_item(Wrapper& entry,
	const std::string& name,
	const std::string& relative,
	const fs::path& absolute,
	const float size,
	std::weak_ptr<editor::asset_directory> opened_dir,
	runtime::asset_manager& manager,
	runtime::input& input, 
	editor::editing_system& edit_state)
{
	auto& selected = edit_state.selection_data.object;
	int action = 0;
	bool already_selected = false;
	if (selected.is_type<Wrapper>())
	{
		if (selected.get_value<Wrapper>() == entry)
		{
			already_selected = true;
		}
	}

	bool is_directory = false;

	if (rttr::type::get<T>() == rttr::type::get<editor::asset_directory>())
		is_directory = true;

	bool edit_label = false;
	if (already_selected && !gui::IsAnyItemActive())
	{
		if (input.is_key_pressed(mml::keyboard::F2))
		{
			edit_label = true;
		}

		if (input.is_key_pressed(mml::keyboard::Delete))
		{
			if (is_directory)
			{
                fs::error_code err;
                fs::remove_all(absolute, err);
			}
			else
			{
				manager.delete_asset<T>(relative);

				if (!opened_dir.expired())
				{
					auto opened_folder_shared = opened_dir.lock();
					auto& files = opened_folder_shared->files;

                    fs::error_code err;
					for (auto& file : files)
					{
						if (file.relative == relative)
						{
                            fs::remove(file.absolute, err);
						}
					}
				}
			}
			edit_state.unselect();
		}
	}

	bool loading = !entry;

	asset_handle<texture>& icon = get_icon();

	if (loading)
		icon = get_loading_icon();
	else
		icon = get_asset_icon(entry);

	gui::PushID(relative.c_str());

	if (gui::GetContentRegionAvailWidth() < size)
		gui::NewLine();

	static std::string inputBuff(64, 0);
	std::memset(&inputBuff[0], 0, 64);
	std::memcpy(&inputBuff[0], name.c_str(), name.size() < 64 ? name.size() : 64);

	ImVec2 item_size = { size, size };
	ImVec2 texture_size = item_size;
	if (icon)
		texture_size = { float(icon->info.width), float(icon->info.height) };
	ImVec2 uv0 = { 0.0f, 0.0f };
	ImVec2 uv1 = { 1.0f, 1.0f };

	bool* edit = edit_label ? &edit_label : nullptr;
	action = gui::ImageButtonWithAspectAndLabel(
		icon.link->asset,
		texture_size, item_size, uv0, uv1,
		already_selected,
		edit,
		loading ? "Loading" : name.c_str(),
		&inputBuff[0],
		inputBuff.size(),
		ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

	if (loading)
	{
		gui::PopID();
		gui::SameLine();
		return action;
	}

	if (action == 1)
	{
		edit_state.select(entry);
	}
	else if (action == 2)
	{
		std::string new_name = std::string(inputBuff.c_str());
		if (new_name != name && new_name != "")
		{
			if (is_directory)
			{
				fs::path new_absolute_path = absolute;
				new_absolute_path.remove_filename();
				new_absolute_path /= new_name;
                fs::error_code err;
                fs::rename(absolute, new_absolute_path, err);
			}
			else
			{
				const auto asset_dir = fs::path(relative).remove_filename();
				std::string new_relative = (asset_dir / new_name).generic_string();
				manager.rename_asset<T>(relative, new_relative);
			}
		}
	}

	if (gui::IsItemHoveredRect())
	{
		if (gui::IsMouseClicked(gui::drag_button) && !edit_state.drag_data.object)
		{
			edit_state.drag(entry, relative);
		}

	}


	gui::PopID();
	gui::SameLine();

	return action;
};


void list_dir(std::weak_ptr<editor::asset_directory>& opened_dir, const float size)
{
	if (opened_dir.expired())
		return;

	auto dir = opened_dir.lock().get();
	
	auto& es = core::get_subsystem<editor::editing_system>();
	auto& am = core::get_subsystem<runtime::asset_manager>();
	auto& input = core::get_subsystem<runtime::input>();
	{
		std::unique_lock<std::mutex> lock(dir->directories_mutex);

		for (auto& entry : dir->directories)
		{
			using entry_t = std::shared_ptr<editor::asset_directory>;
			const auto& name = entry->name;
			const auto& relative = entry->relative;
			const auto& absolute = entry->absolute;
			auto& selected = es.selection_data.object;
			bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
			bool is_dragging = !!es.drag_data.object;
			list_entry(
			entry,
			name,
			is_selected,
			is_dragging,
			size,
			[&]() // on_click
			{
				es.select(entry);
			},
			[&]() // on_double_click
			{
				opened_dir = entry;
				es.try_unselect<std::shared_ptr<editor::asset_directory>>();
			}, 
			[&](const std::string& new_name) // on_rename
			{
				fs::path new_absolute_path = absolute;
				new_absolute_path.remove_filename();
				new_absolute_path /= new_name;
				fs::error_code err;
				fs::rename(absolute, new_absolute_path, err);
			}, 
			[&]() // on_delete
			{
				fs::error_code err;
				fs::remove_all(absolute, err);
			}, 
			[]() // on_drag
			{
			}
			);
		}
	}
	{
		std::unique_lock<std::mutex> lock(dir->files_mutex);

		for (auto& file : dir->files)
		{
			if (file.extension == extensions::texture)
			{
                using asset_t = texture;
                using entry_t = asset_handle<asset_t>;
                auto entry = am.find_or_create_asset_entry<asset_t>(file.relative).asset;
				const auto& name = file.name;
				const auto& relative = file.relative;
				const auto& absolute = file.absolute;
				auto& selected = es.selection_data.object;
				bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
				bool is_dragging = !!es.drag_data.object;
				list_entry(
				entry,
				name,
				is_selected,
				is_dragging,
				size,
				[&]() // on_click
				{
					es.select(entry);
				},
				[&]() // on_double_click
				{
					
				}, 
				[&](const std::string& new_name) // on_rename
				{
					const auto asset_dir = fs::path(relative).remove_filename();
					std::string new_relative = (asset_dir / new_name).generic_string();
                    am.rename_asset<asset_t>(relative, new_relative);
				}, 
				[&]() // on_delete
				{
                    am.delete_asset<asset_t>(relative);

					if (!opened_dir.expired())
					{
						auto opened_folder_shared = opened_dir.lock();
						auto& files = opened_folder_shared->files;

						fs::error_code err;
						for (auto& f : files)
						{
							if (f.relative == relative)
							{
								fs::remove(f.absolute, err);
							}
						}
					}
				}, 
				[&]() // on_drag
				{
					es.drag(entry, relative);
				}
				);
			}
			if (file.extension == extensions::mesh)
			{
                using asset_t = mesh;
                using entry_t = asset_handle<asset_t>;
                auto entry = am.find_or_create_asset_entry<asset_t>(file.relative).asset;
                const auto& name = file.name;
                const auto& relative = file.relative;
                const auto& absolute = file.absolute;
                auto& selected = es.selection_data.object;
                bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
                bool is_dragging = !!es.drag_data.object;
                list_entry(
                entry,
                name,
                is_selected,
                is_dragging,
                size,
                [&]() // on_click
                {
                    es.select(entry);
                },
                [&]() // on_double_click
                {

                },
                [&](const std::string& new_name) // on_rename
                {
                    const auto asset_dir = fs::path(relative).remove_filename();
                    std::string new_relative = (asset_dir / new_name).generic_string();
                    am.rename_asset<asset_t>(relative, new_relative);
                },
                [&]() // on_delete
                {
                    am.delete_asset<asset_t>(relative);

                    if (!opened_dir.expired())
                    {
                        auto opened_folder_shared = opened_dir.lock();
                        auto& files = opened_folder_shared->files;

                        fs::error_code err;
                        for (auto& f : files)
                        {
                            if (f.relative == relative)
                            {
                                fs::remove(f.absolute, err);
                            }
                        }
                    }
                },
                [&]() // on_drag
                {
                    es.drag(entry, relative);
                }
                );
			}
			if (file.extension == extensions::material)
			{
                using asset_t = material;
                using entry_t = asset_handle<asset_t>;
                auto entry = am.find_or_create_asset_entry<asset_t>(file.relative).asset;
                const auto& name = file.name;
                const auto& relative = file.relative;
                const auto& absolute = file.absolute;
                auto& selected = es.selection_data.object;
                bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
                bool is_dragging = !!es.drag_data.object;
                list_entry(
                entry,
                name,
                is_selected,
                is_dragging,
                size,
                [&]() // on_click
                {
                    es.select(entry);
                },
                [&]() // on_double_click
                {

                },
                [&](const std::string& new_name) // on_rename
                {
                    const auto asset_dir = fs::path(relative).remove_filename();
                    std::string new_relative = (asset_dir / new_name).generic_string();
                    am.rename_asset<asset_t>(relative, new_relative);
                },
                [&]() // on_delete
                {
                    am.delete_asset<asset_t>(relative);

                    if (!opened_dir.expired())
                    {
                        auto opened_folder_shared = opened_dir.lock();
                        auto& files = opened_folder_shared->files;

                        fs::error_code err;
                        for (auto& f : files)
                        {
                            if (f.relative == relative)
                            {
                                fs::remove(f.absolute, err);
                            }
                        }
                    }
                },
                [&]() // on_drag
                {
                    es.drag(entry, relative);
                }
                );
			}
            if (file.extension == extensions::shader)
            {
                using asset_t = shader;
                using entry_t = asset_handle<asset_t>;
                auto entry = am.find_or_create_asset_entry<asset_t>(file.relative).asset;
                const auto& name = file.name;
                const auto& relative = file.relative;
                const auto& absolute = file.absolute;
                auto& selected = es.selection_data.object;
                bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
                bool is_dragging = !!es.drag_data.object;
                list_entry(
                entry,
                name,
                is_selected,
                is_dragging,
                size,
                [&]() // on_click
                {
                    es.select(entry);
                },
                [&]() // on_double_click
                {

                },
                [&](const std::string& new_name) // on_rename
                {
                    const auto asset_dir = fs::path(relative).remove_filename();
                    std::string new_relative = (asset_dir / new_name).generic_string();
                    am.rename_asset<asset_t>(relative, new_relative);
                },
                [&]() // on_delete
                {
                    am.delete_asset<asset_t>(relative);

                    if (!opened_dir.expired())
                    {
                        auto opened_folder_shared = opened_dir.lock();
                        auto& files = opened_folder_shared->files;

                        fs::error_code err;
                        for (auto& f : files)
                        {
                            if (f.relative == relative)
                            {
                                fs::remove(f.absolute, err);
                            }
                        }
                    }
                },
                [&]() // on_drag
                {
                    es.drag(entry, relative);
                }
                );
            }
			if (file.extension == extensions::prefab)
			{
                using asset_t = prefab;
                using entry_t = asset_handle<asset_t>;
                auto entry = am.find_or_create_asset_entry<asset_t>(file.relative).asset;
                const auto& name = file.name;
                const auto& relative = file.relative;
                const auto& absolute = file.absolute;
                auto& selected = es.selection_data.object;
                bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
                bool is_dragging = !!es.drag_data.object;
                list_entry(
                entry,
                name,
                is_selected,
                is_dragging,
                size,
                [&]() // on_click
                {
                    es.select(entry);
                },
                [&]() // on_double_click
                {

                },
                [&](const std::string& new_name) // on_rename
                {
                    const auto asset_dir = fs::path(relative).remove_filename();
                    std::string new_relative = (asset_dir / new_name).generic_string();
                    am.rename_asset<asset_t>(relative, new_relative);
                },
                [&]() // on_delete
                {
                    am.delete_asset<asset_t>(relative);

                    if (!opened_dir.expired())
                    {
                        auto opened_folder_shared = opened_dir.lock();
                        auto& files = opened_folder_shared->files;

                        fs::error_code err;
                        for (auto& f : files)
                        {
                            if (f.relative == relative)
                            {
                                fs::remove(f.absolute, err);
                            }
                        }
                    }
                },
                [&]() // on_drag
                {
                    es.drag(entry, relative);
                }
                );
			}
			if (file.extension == extensions::scene)
			{
                using asset_t = scene;
                using entry_t = asset_handle<asset_t>;
                auto entry = am.find_or_create_asset_entry<asset_t>(file.relative).asset;
                const auto& name = file.name;
                const auto& relative = file.relative;
                const auto& absolute = file.absolute;
                auto& selected = es.selection_data.object;
                bool is_selected = selected.is_type<entry_t>() ? (selected.get_value<entry_t>() == entry) : false;
                bool is_dragging = !!es.drag_data.object;
                list_entry(
                entry,
                name,
                is_selected,
                is_dragging,
                size,
                [&]() // on_click
                {
                    es.select(entry);
                },
                [&]() // on_double_click
                {
                    if(!entry)
                        return;

                    auto& ecs = core::get_subsystem<runtime::entity_component_system>();
                    ecs.dispose();
                    es.load_editor_camera();
                    entry->instantiate();
                    es.scene = fs::resolve_protocol(entry.id()).string() + extensions::scene;


                },
                [&](const std::string& new_name) // on_rename
                {
                    const auto asset_dir = fs::path(relative).remove_filename();
                    std::string new_relative = (asset_dir / new_name).generic_string();
                    am.rename_asset<asset_t>(relative, new_relative);
                },
                [&]() // on_delete
                {
                    am.delete_asset<asset_t>(relative);

                    if (!opened_dir.expired())
                    {
                        auto opened_folder_shared = opened_dir.lock();
                        auto& files = opened_folder_shared->files;

                        fs::error_code err;
                        for (auto& f : files)
                        {
                            if (f.relative == relative)
                            {
                                fs::remove(f.absolute, err);
                            }
                        }
                    }
                },
                [&]() // on_drag
                {
                    es.drag(entry, relative);
                }
                );

			}

		}
	}

	if (gui::BeginPopupContextWindow())
	{
		if (gui::BeginMenu("Create"))
		{
			if (gui::MenuItem("Folder"))
			{
				auto opened_folder_shared = opened_dir.lock();
				auto dir = opened_folder_shared.get();
				if (dir)
				{
					int i = 0;
                    fs::error_code err;
                    while (!fs::create_directory(dir->absolute / string_utils::format("New Folder (%d)", i), err))
					{
						++i;
					}
				}
				
			}

			gui::Separator();

			if (gui::MenuItem("Material"))
			{
				auto opened_folder_shared = opened_dir.lock();
				auto dir = opened_folder_shared.get();
				if (dir)
				{
					asset_handle<material> asset;
					fs::path parent_dir = dir->relative;
					asset.link->id = (parent_dir / string_utils::format("New Material (%s)", fs::path(std::tmpnam(nullptr)).filename().string().c_str())).generic_string();
					asset.link->asset = std::make_shared<standard_material>();
					am.save<material>(asset);
				}
				
			}

			gui::EndMenu();
		}

		gui::Separator();

		if (gui::Selectable("Open In Explorer"))
		{
			auto opened_folder_shared = opened_dir.lock();
			auto dir = opened_folder_shared.get();
			if(dir)
				fs::show_in_graphical_env(dir->absolute);
		}

		gui::EndPopup();
	}

}

void assets_dock::render(const ImVec2& area)
{
	auto& project = core::get_subsystem<editor::project_manager>();

	if (opened_dir.expired())
		opened_dir = project.get_root_directory();

	auto& es = core::get_subsystem<editor::editing_system>();
	auto& input = core::get_subsystem<runtime::input>();

	if (!gui::IsAnyItemActive())
	{
		if (input.is_key_pressed(mml::keyboard::BackSpace) && !opened_dir.expired())
		{
			auto opened_folder_shared = opened_dir.lock();

			if (opened_folder_shared->parent)
				opened_dir = opened_folder_shared->parent->shared_from_this();
		}
	}


	if (gui::Button("Import..."))
	{
		std::vector<std::string> paths;
		if (open_multiple_files_dialog("obj,fbx,dae,blend,3ds,mtl,png,jpg,tga,dds,ktx,pvr,sc,io,sh", "", paths))
		{
			auto& ts = core::get_subsystem<runtime::task_system>();

			auto opened_folder_shared = opened_dir.lock();

			fs::path opened_dir = opened_folder_shared->absolute;
			for (auto& path : paths)
			{
				fs::path p = path;
				fs::path ext = p.extension().string();
				fs::path filename = p.filename();

				auto task = ts.create("Import Asset", [opened_dir](const fs::path& path, const fs::path& p, const fs::path& filename)
				{
                    fs::error_code err;
					fs::path dir = opened_dir / filename;
                    fs::copy_file(path, dir, err);
					//{
					//	APPLOG_ERROR("Failed to import file {0} with message {1}", p.string(), error.message());
					//}
					//else
					{
                        fs::last_write_time(dir, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), err);
					}
				}, p, p, filename);

				ts.run(task);
			}
		}
	}
	gui::SameLine();
	gui::PushItemWidth(80.0f);
	gui::SliderFloat("", &scale_icons, 0.5f, 1.0f);
	if (gui::IsItemHovered())
	{
		gui::SetTooltip("Scale Icons");
	}
	gui::PopItemWidth();
	gui::SameLine();

	std::vector<editor::asset_directory*> hierarchy;
	auto dir = opened_dir.lock().get();
	auto f = dir;
	while (f)
	{
		hierarchy.push_back(f);
		f = f->parent;
	}

	for (auto rit = hierarchy.rbegin(); rit != hierarchy.rend(); ++rit)
	{
		if (rit != hierarchy.rbegin())
		{
			gui::Text(">");
			gui::SameLine();
		}

		if (gui::Button((*rit)->name.c_str()))
		{
			opened_dir = (*rit)->shared_from_this();
			break;
		}
		if (rit != hierarchy.rend() - 1)
			gui::SameLine();
	}
	gui::Separator();

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings;
	if (gui::BeginChild("assets_content", gui::GetContentRegionAvail(), false, flags))
	{
		if (gui::IsWindowHovered())
		{
			auto& dragged = es.drag_data.object;
			if (dragged && dragged.is_type<runtime::entity>())
			{
				gui::SetMouseCursor(ImGuiMouseCursor_Move);
				if (gui::IsMouseReleased(gui::drag_button))
				{
					auto entity = dragged.get_value<runtime::entity>();
					if (entity)
						ecs::utils::save_entity(dir->absolute, entity);
					es.drop();
				}
			}
		}

		get_icon() = asset_handle<texture>();
		list_dir(opened_dir, 88.0f * scale_icons);
		get_icon() = asset_handle<texture>();
		gui::EndChild();
	}

}

assets_dock::assets_dock(const std::string& dtitle, bool dcloseButton, ImVec2 dminSize)
{
	initialize(dtitle, dcloseButton, dminSize, std::bind(&assets_dock::render, this, std::placeholders::_1));
}
