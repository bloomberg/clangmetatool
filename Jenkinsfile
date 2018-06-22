library identifier: 'blp-dpkg-jaas-library@master', retriever: modernSCM([
    $class: 'GitSCMSource',
    credentialsId: 'bbgithub_token',
    remote: 'https://bbgithub.dev.bloomberg.com/dpkg/blp-dpkg-jaas-library'
])

blpDpkgBuildSinglePackage(
    scm: scm,
    nodes: ['BLDRH6', 'BLDSUN']
)
