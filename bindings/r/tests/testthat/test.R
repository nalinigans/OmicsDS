library(omicsds)

test_that("version is valid", {
    version <- omicsds::version()
    print(version)
    expect_vector(version, ptype = character())
    expect_match(version, c("\\d*.\\d*.\\d*"))
})

test_that("test that omicsds connects to an existing workspace for queries", {
    print(getwd())
    omicsds_repository <- Sys.getenv(c("OMICSDS_REPOSITORY"), unset="../../../..")
    root_dir <- Sys.getenv(c("GITHUB_WORKSPACE"), omicsds_repository)
    omicsds_handle <- omicsds::connect(workspace=paste(root_dir, "src/test/inputs/feature-level-ws", sep="/"), array="array")
    df <- omicsds::query_features(handle=omicsds_handle, features=c("ENSG00000138190", "ENSG00000243485"), sample_range=c(0, 2))
    omicsds::disconnect(handle=omicsds_handle)
    expected_df = data.frame(c(405,2301),c(177,153),c(1488,828))
    colnames(expected_df) <- c("2", "1", "0")
    rownames(expected_df)<- c("ENSG00000243485", "ENSG00000138190")
    expect_true(identical(df, expected_df))
})

