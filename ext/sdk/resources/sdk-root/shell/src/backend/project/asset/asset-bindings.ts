import { interfaces } from "inversify";
import { bindContributionProvider } from "backend/contribution-provider";
import { AssetManagerContribution, bindAssetContribution } from "./asset-manager-contribution";
import { AssetImporterContribution, bindAssetImporterContribution } from "./asset-importer-contribution";
import { assetImporterTypes } from "shared/asset.types";
import { ENABLED_ASSET_MANAGERS } from "assets/enabled-managers";
import { GitImporter } from "./importer-contributions/git-importer/git-importer";
import { FsImporter } from "./importer-contributions/fs-importer/fs-importer";
import { ReleaseImporter } from "./importer-contributions/release-importer/release-importer";
import { ExampleImporter } from "./importer-contributions/example-importer/example-importer";

export const bindAsset = (container: interfaces.Container) => {
  bindContributionProvider(container, AssetManagerContribution);
  bindContributionProvider(container, AssetImporterContribution);

  container.bind(GitImporter).toSelf().inSingletonScope();
  bindAssetImporterContribution(container, GitImporter, assetImporterTypes.git);

  container.bind(FsImporter).toSelf().inSingletonScope();
  bindAssetImporterContribution(container, FsImporter, assetImporterTypes.fs);

  container.bind(ReleaseImporter).toSelf().inSingletonScope();
  bindAssetImporterContribution(container, ReleaseImporter, assetImporterTypes.release);

  container.bind(ExampleImporter).toSelf().inSingletonScope();
  bindAssetImporterContribution(container, ExampleImporter, assetImporterTypes.example);

  for (const [assetType, AssetManager] of Object.entries(ENABLED_ASSET_MANAGERS)) {
    container.bind<AssetManagerContribution>(AssetManager).toSelf().inSingletonScope();
    bindAssetContribution<AssetManagerContribution>(container, AssetManager, assetType);
  }
};
